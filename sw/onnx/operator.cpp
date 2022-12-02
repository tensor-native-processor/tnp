#include "operator.h"
#include "logging.h"
#include "error.h"

#include "operators/gemm.h"
#include "operators/relu.h"

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
        std::map<std::string, Tensor::Shape>& state_shape) {
    Operator* op = OperatorDispatch::createOperator(node.op_type());
    op->inferShape(node, state_initializer, state_shape);
    delete op;
}

// Dispatch simulate
void Operator::simulate(const ::onnx::NodeProto& node,
        std::map<std::string, Tensor>& state_tensor) {
    Operator* op = OperatorDispatch::createOperator(node.op_type());
    op->simulate(node, state_tensor);
    delete op;
}
