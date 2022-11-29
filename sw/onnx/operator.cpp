#include "operator.h"
#include "logging.h"
#include "error.h"

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
}


// InferShape for Relu
void OperatorRelu::inferShape(const ::onnx::NodeProto& node,
        const std::map<std::string, Tensor>& state_initializer,
        std::map<std::string, Shape>& state_shape) const {

    LogWarning("Node " + node.name());
}
