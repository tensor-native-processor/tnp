#include "operators/relu.h"
#include "error.h"
#include "logging.h"

// InferShape for Relu
void OperatorRelu::inferShape(const ::onnx::NodeProto& node,
        const std::map<std::string, Tensor>& state_initializer,
        std::map<std::string, Shape>& state_shape) {

    // Validate i/o size
    if (node.input_size() != 1 || node.output_size() != 1) {
        FatalError("Relu io size not 1");
    }

    // Echo shape
    if (state_shape.count(node.input(0)) == 0) {
        FatalError("Gemm cannot determine shape X " + node.input(0));
    }
    Shape shapeX = state_shape.at(node.input(0));
    state_shape[node.output(0)] = shapeX;
}

// Simulate for Relu
void OperatorRelu::simulate(const ::onnx::NodeProto& node,
        std::map<std::string, Tensor>& state_tensor) {

    LogWarning("Simulate " + node.name());

    // Validate i/o size
    if (node.input_size() != 1 || node.output_size() != 1) {
        FatalError("Relu io size not 1");
    }

    // Fetch input tensor
    if (state_tensor.count(node.input(0)) == 0) {
        FatalError("Relu cannot find input tensor");
    }
    const Tensor& in_tensor = state_tensor.at(node.input(0));
    Tensor out_tensor(in_tensor.m_shape);
    for (size_t i = 0;i < in_tensor.m_size;i++) {
        out_tensor.m_value[i] = relu(in_tensor.m_value[i]);
    }
    state_tensor.emplace(node.output(0), out_tensor);
}

// Relu function
float OperatorRelu::relu(float x) const {
    return x > 0.0f ? x : 0.0f;
}
