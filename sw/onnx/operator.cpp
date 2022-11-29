#include "operator.h"
#include "logging.h"
#include "error.h"
#include <iostream>

// Operator directory
const std::map<std::string, Operator*(*)()> OperatorDispatch::g_operatorDirectory = {
    {"Gemm", createOperatorImpl<OperatorGemm>},
    {"Relu", createOperatorImpl<OperatorRelu>},
};

// Dispatch to correct operator
Operator* OperatorDispatch::createOperator(const std::string& name) {
    return g_operatorDirectory.at(name)();
}
template<class T>
Operator* OperatorDispatch::createOperatorImpl() {
    return new T;
}


// Dispatch inferShape
void Operator::inferShape(const ::onnx::NodeProto& node,
        const std::map<std::string, Tensor>& state_initializer,
        std::map<std::string, Shape>& state_shape) const {
    Operator* op = OperatorDispatch::createOperator(node.op_type());
    op->inferShape(node, state_initializer, state_shape);
    delete op;
}


// InferShape for Gemm
void OperatorGemm::inferShape(const ::onnx::NodeProto& node,
        const std::map<std::string, Tensor>& state_initializer,
        std::map<std::string, Shape>& state_shape) const {

    LogWarning("Node " + node.name());

    // Fetch A and B
    if (node.input_size() != 2 && node.input_size() != 3) {
        FatalError("Gemm input size " + std::to_string(node.input_size()));
    }
    Shape shapeA, shapeB;
    if (state_shape.count(node.input(0)) == 0 ||
        state_shape.count(node.input(1)) == 0) {
        FatalError("Gemm cannot determine shape A/B");
    }
    shapeA = state_shape.at(node.input(0));
    shapeB = state_shape.at(node.input(1));
    if (shapeA.size() != 2 || shapeB.size() != 2) {
        FatalError("Gemm input dimension not 2D");
    }
    std::cerr << shapeA.size() << " " << shapeB.size() << std::endl;

}


// InferShape for Relu
void OperatorRelu::inferShape(const ::onnx::NodeProto& node,
        const std::map<std::string, Tensor>& state_initializer,
        std::map<std::string, Shape>& state_shape) const {

    LogWarning("Node " + node.name());
}
