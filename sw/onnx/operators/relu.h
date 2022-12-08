#ifndef __OPERATOR_RELU_H__
#define __OPERATOR_RELU_H__

#include "operator.h"

// Relu
class OperatorRelu: public Operator {
public:
    void inferShape(const ::onnx::NodeProto&,
        const std::map<std::string, Tensor>&,
        std::map<std::string, Tensor::Shape>&) override;

    void simulate(const ::onnx::NodeProto&,
        std::map<std::string, Tensor>&) override;

    void compile(const ::onnx::NodeProto&,
        Orchestrator&,
        std::map<std::string, Orchestrator::MatrixHandle>&) override;

private:
    float relu(float) const;
};

#endif
