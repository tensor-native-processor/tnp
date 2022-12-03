#include "operators/relu.h"
#include "error.h"
#include "logging.h"

// InferShape for Relu
void OperatorRelu::inferShape(const ::onnx::NodeProto& node,
        const std::map<std::string, Tensor>& stateInitializer,
        std::map<std::string, Tensor::Shape>& stateShape) {

    // Validate i/o size
    if (node.input_size() != 1 || node.output_size() != 1) {
        FatalError("Relu io size not 1");
    }

    // Echo shape
    if (stateShape.count(node.input(0)) == 0) {
        FatalError("Gemm cannot determine shape X " + node.input(0));
    }
    Tensor::Shape shapeX = stateShape.at(node.input(0));
    stateShape[node.output(0)] = shapeX;
}

// Simulate for Relu
void OperatorRelu::simulate(const ::onnx::NodeProto& node,
        std::map<std::string, Tensor>& stateTensor) {

    // Validate i/o size
    if (node.input_size() != 1 || node.output_size() != 1) {
        FatalError("Relu io size not 1");
    }

    // Fetch input tensor
    if (stateTensor.count(node.input(0)) == 0) {
        FatalError("Relu cannot find input tensor");
    }
    const Tensor& in_tensor = stateTensor.at(node.input(0));
    Tensor out_tensor(in_tensor.m_shape);
    for (size_t i = 0;i < in_tensor.m_size;i++) {
        out_tensor.m_value[i] = relu(in_tensor.m_value[i]);
    }
    stateTensor.emplace(node.output(0), out_tensor);
}

// Relu function
float OperatorRelu::relu(float x) const {
    return x > 0.0f ? x : 0.0f;
}
