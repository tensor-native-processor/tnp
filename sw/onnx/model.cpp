#include "model.h"
#include "tensor.h"
#include "logging.h"
#include "error.h"
#include "operator.h"
#include "orchestration.h"

#include <queue>
#include <fstream>
#include <iostream>


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
    inferShape();
}


// Generate shape for each variable
void ONNXModel::inferShape() {
    std::map<std::string, Tensor::Shape> stateShapes;
    for (const auto& [name, init] : m_initializers) {
        stateShapes[name] = init.m_shape;
    }
    for (const auto& [name, in] : m_inputs) {
        stateShapes[name] = in;
    }

    // Determine shape for each node
    Operator op;
    for (const auto& node : m_graph.node()) {
        LogInfo(node.name() + " (" + node.op_type() + ")");

        op.inferShape(node, m_initializers, stateShapes);
    }

    // Print output shape
    for (const auto& out : m_outputs) {
        std::string msg = "  o " + out + ":";
        if (stateShapes.count(out) == 0) {
            FatalError("No output shape " + out);
        }
        for (const auto& d : stateShapes.at(out)) {
            msg += " " + std::to_string(d);
        }
        LogInfo(msg);
    }
}


// Simulate model
std::vector<Tensor> ONNXModel::simulate(const std::vector<Tensor>& inputTensors) {
    std::map<std::string, Tensor> stateTensors(m_initializers);

    // Import inputTensor into stateTensor
    if (inputTensors.size() < (size_t)m_graph.input_size()) {
        FatalError("Insufficient inputs for simulate");
    }
    for (size_t i = 0;i < (size_t)m_graph.input_size();i++) {
        stateTensors.emplace(m_graph.input(i).name(), inputTensors[i]);
    }

    // Simulate each operator
    Operator op;
    for (const auto& node : m_graph.node()) {
        LogInfo("Simulate: " + node.name());
        op.simulate(node, stateTensors);
    }

    // Output tensor
    std::vector<Tensor> outputTensors;
    for (size_t i = 0;i < (size_t)m_graph.output_size();i++) {
        if (stateTensors.count(m_graph.output(i).name()) == 0) {
            FatalError("No output " + m_graph.output(i).name());
        }
        outputTensors.emplace_back(stateTensors.at(m_graph.output(i).name()));
    }
    return outputTensors;
}

// Compile model into TNP binaries
void ONNXModel::compile(const OrchestratorParam& orchParam, const std::vector<Tensor>& inputTensors) {
    Orchestrator orch(orchParam);
    std::map<std::string, Orchestrator::MatrixHandle> stateTensorHandles;

    // Import initializers as handles
    for (const auto& [name, init] : m_initializers) {
        auto mc = init.toMatrixConstant(orchParam.width);
        auto handle = orch.dataMatrixAllocate(mc.m_shape);
        orch.dataMatrixLoadConstant(handle, mc);
        stateTensorHandles[name] = handle;
    }

    // Import input as handles
    if (inputTensors.size() < (size_t)m_graph.input_size()) {
        FatalError("Insufficient inputs for simulate");
    }
    for (size_t i = 0;i < (size_t)m_graph.input_size();i++) {
        auto mc = inputTensors[i].toMatrixConstant(orchParam.width);
        auto handle = orch.dataMatrixAllocate(mc.m_shape);
        orch.dataMatrixLoadConstant(handle, mc);
        stateTensorHandles[m_graph.input(i).name()] = handle;
    }

    // Simulate each operator
    Operator op;
    for (const auto& node : m_graph.node()) {
        LogInfo("Compiling: " + node.name());
        // op.compile(node, orch, stateTensorHandles);
    }

    // Output tensor
    std::vector<Tensor> outputTensors;
    for (size_t i = 0;i < (size_t)m_graph.output_size();i++) {
        if (stateTensorHandles.count(m_graph.output(i).name()) == 0) {
            FatalError("No output " + m_graph.output(i).name());
        }
        auto handle = stateTensorHandles.at(m_graph.output(i).name()); 
        orch.dataMatrixStoreResult(handle);
    }
}
