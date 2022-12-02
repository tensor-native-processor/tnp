#include "model.h"
#include "logging.h"
#include "error.h"
#include "operator.h"

#include <queue>
#include <fstream>

// Construct Tensor from protobuf
Tensor::Tensor(const ::onnx::TensorProto& tensor) {
    // Name
    m_name = tensor.name();

    // Dimension
    m_size = 1;
    for (auto dim : tensor.dims()) {
        m_shape.push_back(dim);
        m_size *= dim;
    }
    m_value = std::make_unique<float[]>(m_size);
    std::fill_n(m_value.get(), m_size, 0);

    // Value
    switch (tensor.data_type()) {
    case ::onnx::TensorProto::FLOAT: {
        if (tensor.float_data_size() != 0) {
            // Float data
            if ((size_t)tensor.float_data_size() > m_size) {
                FatalError("Initializer " + m_name + " larger than dimension " + std::to_string(m_size));
            }
            for (size_t i = 0;i < (size_t)tensor.float_data_size();i++) {
                m_value[i] = tensor.float_data(i);
            }
        } else {
            // Raw data
            std::string bytes = tensor.raw_data();
            if (bytes.size() / sizeof(float) > m_size) {
                FatalError("Initializer " + m_name + " larger than dimension " + std::to_string(m_size));
            }
            std::copy_n(bytes.c_str(), bytes.size(), (char*)m_value.get());
        }
        break;
    }
    case ::onnx::TensorProto::INT64: {
        if (tensor.int64_data_size() != 0) {
            if ((size_t)tensor.int64_data_size() > m_size) {
                FatalError("Initializer " + m_name + " larger than dimension " + std::to_string(m_size));
            }
            for (size_t i = 0;i < (size_t)tensor.int64_data_size();i++) {
                // Casted to float for now
                m_value[i] = (float)tensor.int64_data(i);
            }
        } else {
            LogWarning("Initializer tensor " + m_name + " int64 raw data");
        }
        break;
    }
    default: {
        LogWarning("Initializer tensor " + m_name + " has unsupported type " + std::to_string(tensor.data_type()) + " and set to 0");
    }
    }
}

// Construct Tensor from shape
Tensor::Tensor(const Shape& shape)
: m_shape(shape) {
    // Calculate size
    m_size = 1;
    for (auto dim : m_shape) {
        m_size *= dim;
    }
    // Allocate value
    m_value = std::make_unique<float[]>(m_size);
    std::fill_n(m_value.get(), m_size, 0);
}

// Copy-construct tensor
Tensor::Tensor(const Tensor& tensor)
: m_name(tensor.m_name), m_shape(tensor.m_shape), m_size(tensor.m_size) {
    // Allocate value
    m_value = std::make_unique<float[]>(m_size);

    // Copy from tensor
    std::copy_n(tensor.m_value.get(), m_size, m_value.get());
}


// Locate an index in tensor
const float& Tensor::locate(const Index& idx) const {
    if (idx.size() != m_shape.size()) {
        FatalError("Tensor index mismatch shape");
    }

    size_t decode = 0, block = 1;
    for (ssize_t pos = m_shape.size() - 1;pos >= 0;pos--) {
        decode += idx[pos] * block;
        block *= m_shape[pos];
    }
    return m_value[decode];
}
float& Tensor::locate(const Index& idx) {
    return const_cast<float&>(const_cast<const Tensor*>(this)->locate(idx));
}


// Constructor for ONNXModel
ONNXModel::ONNXModel(const std::string& filename) {
    LogInfo("Loading model " + filename);

    // Find file size
    FILE* file = fopen(filename.c_str(), "rb");
    if (file == NULL) {
        FatalError(strerror(errno));
    }
    fseek(file, 0, SEEK_END);
    size_t filesize = ftell(file);
    rewind(file);

    // Read into buffer
    void* buffer = malloc(filesize);
    fread(buffer, filesize, 1, file);
    fclose(file);

    // Construct model object
    bool ret = m_model.ParseFromArray(buffer, filesize);
    if (!ret) {
        FatalError("Cannot load model " + filename);
    }

    // Free buffer
    free(buffer);

    // Other init operation
    loadModel();
}

// Loading data from model
void ONNXModel::loadModel() {
    m_graph = m_model.graph();
    LogInfo("IR version " + std::to_string(m_model.ir_version()));
    LogInfo("Graph name " + m_graph.name());

    // Initializer
    for (const auto& init : m_graph.initializer()) {
        m_initializers.emplace(init.name(), init);
    }

    // Input/output
    for (const auto& in : m_graph.input()) {
        Tensor::Shape cur_shape;

        // Get tensor type
        const auto& type = in.type();
        if (type.value_case() != ::onnx::TypeProto::ValueCase::kTensorType) {
            LogWarning("Input " + in.name() + " is not tensor (ignored)");
            continue;
        }

        // Print overall dimension
        std::string dim_str = "";
        for (const auto& dim : type.tensor_type().shape().dim()) {
            switch (dim.value_case()) {
            case ::onnx::TensorShapeProto::Dimension::ValueCase::kDimParam: {
                dim_str += dim.dim_param() + " ";
                LogWarning("Tensor input " + in.name() + " dim param " + dim.dim_param() + " skipped");
                break;
            }
            case ::onnx::TensorShapeProto::Dimension::ValueCase::kDimValue: {
                cur_shape.push_back(dim.dim_value());
                dim_str += std::to_string(dim.dim_value()) + " ";
                break;
            }
            default: {
                FatalError("Tensor input " + in.name() + " has no dimension");
            }
            }
        }
        LogInfo("Input " + in.name() + " dim: " + dim_str);
        m_inputs[in.name()] = cur_shape;
    }
    // Outputs
    for (const auto& out : m_graph.output()) {
        m_outputs.push_back(out.name());
    }

    // Load shapes
    genShape();
}


// Generate shape for each variable
void ONNXModel::genShape() {
    std::map<std::string, Tensor::Shape> shape;
    for (const auto& [name, init] : m_initializers) {
        shape[name] = init.m_shape;
    }
    for (const auto& [name, in] : m_inputs) {
        shape[name] = in;
    }

    // Determine shape for each node
    Operator op;
    for (const auto& node : m_graph.node()) {
        LogInfo(node.name() + " (" + node.op_type() + ")");

        op.inferShape(node, m_initializers, shape);
    }

    // Print output shape
    for (const auto& out : m_outputs) {
        std::string msg = "  o " + out + ":";
        if (shape.count(out) == 0) {
            FatalError("No output shape " + out);
        }
        for (const auto& d : shape.at(out)) {
            msg += " " + std::to_string(d);
        }
        LogInfo(msg);
    }
}


// Simulate model
void ONNXModel::simulate() {
    std::map<std::string, Tensor> stateTensor(m_initializers);

    // MNIST-specific input
    std::string inputName = m_graph.input(0).name();
    std::ifstream inputFile("X.txt");
    Tensor inputTensor({10, 400});
    for (size_t b = 0;b < 10;b++) {
        for (size_t i = 0;i < 400;i++) inputFile >> inputTensor.locate({b, i});
    }
    inputFile.close();
    stateTensor.emplace(inputName, inputTensor);

    // Simulate each operator
    Operator op;
    for (const auto& node : m_graph.node()) {
        op.simulate(node, stateTensor);
    }

    // MNIST-specific output
    std::string outputName = m_graph.output(0).name();
    const Tensor& outputTensor = stateTensor.at(outputName);
    for (size_t b = 0;b < 10;b++) {
        for (size_t i = 0;i < 10;i++) std::cout << outputTensor.locate({b, i}) << ", ";
        std::cout << std::endl;
    }
}
