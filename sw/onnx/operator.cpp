#include "operator.h"

// Operator directory
const std::map<std::string, const Operator&> g_operatorDirectory = {
    {"Gemm", OperatorGemm{}}
};


// Dispatch inferShape
void Operator::inferShape(const ::onnx::NodeProto& node,
        const std::map<std::string, Tensor>& state_initializer,
        std::map<std::string, Shape>& state_shape) const {
    g_operatorDirectory.at(node.op_type()).inferShape(node, state_initializer, state_shape);
}


// InferShape for Gemm
void OperatorGemm::inferShape(const ::onnx::NodeProto& node,
        const std::map<std::string, Tensor>& state_initializer,
        std::map<std::string, Shape>& state_shape) const {

}
