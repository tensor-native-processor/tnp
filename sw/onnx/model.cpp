#include "model.h"
#include "logging.h"
#include "error.h"

#include <queue>

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
    m_value = (float*)calloc(m_size, sizeof(float));
    if (m_value == NULL) {
        FatalError("Insufficient memory");
    }

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
            memcpy(m_value, bytes.c_str(), bytes.size());
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

// Cleanup tensor
Tensor::~Tensor() {
    free(m_value);
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
        m_inputs[in.name()] = Shape{};

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
                break;
            }
            case ::onnx::TensorShapeProto::Dimension::ValueCase::kDimValue: {
                dim_str += std::to_string(dim.dim_value()) + " ";
                break;
            }
            default: {
                FatalError("Tensor input " + in.name() + " has no dimension");
            }
            }
        }
        LogInfo("Input " + in.name() + " dim: " + dim_str);
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
    std::map<std::string, Shape> shape;
    for (const auto& [name, init] : m_initializers) {
        shape[name] = init.m_shape;
    }
    for (const auto& [name, in] : m_inputs) {
        shape[name] = in;
    }

    // Determine shape for each node
    for (const auto& node : m_graph.node()) {
        LogInfo(node.name() + " (" + node.op_type() + ")");

        for (const auto& in : node.input()) {
            if (shape.count(in) == 0) {
                FatalError("Unknown input " + in);
            }
        }
        for (const auto& out : node.output()) {
            shape[out] = Shape{};
        }
    }
}
