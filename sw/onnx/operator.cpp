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
        const std::map<std::string, Tensor>& stateInitializer,
        std::map<std::string, Tensor::Shape>& stateShape) {
    Operator* op = OperatorDispatch::createOperator(node.op_type());
    op->inferShape(node, stateInitializer, stateShape);
    delete op;
}

// Dispatch simulate
void Operator::simulate(const ::onnx::NodeProto& node,
        std::map<std::string, Tensor>& stateTensor) {
    Operator* op = OperatorDispatch::createOperator(node.op_type());
    op->simulate(node, stateTensor);
    delete op;
}

// Displatch compile
void Operator::compile(const ::onnx::NodeProto& node,
        Orchestrator& orch,
        std::map<std::string, Orchestrator::MatrixHandle>& stateTensorHandles) {
    Operator* op = OperatorDispatch::createOperator(node.op_type());
    op->compile(node, orch, stateTensorHandles);
    delete op;
}
