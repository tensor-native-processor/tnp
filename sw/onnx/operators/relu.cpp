#include "operators/relu.h"
#include "error.h"
#include "logging.h"

// InferShape for Relu
void OperatorRelu::inferShape(const ::onnx::NodeProto& node,
        const std::map<std::string, Tensor>& state_initializer,
        std::map<std::string, Shape>& state_shape) const {

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
        std::map<std::string, Tensor>& state_tensor) const {

    LogWarning("Simulate " + node.name());
}
