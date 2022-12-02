#include "model.h"
#include "tensor.h"
#include "logging.h"
#include "error.h"
#include "operator.h"

#include <queue>
#include <fstream>


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
