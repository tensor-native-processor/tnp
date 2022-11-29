#ifndef __OPERATOR_RELU_H__
#define __OPERATOR_RELU_H__

#include "operator.h"

// Relu
class OperatorRelu: public Operator {
public:
    void inferShape(const ::onnx::NodeProto&,
        const std::map<std::string, Tensor>&,
        std::map<std::string, Shape>&) override;

    void simulate(const ::onnx::NodeProto&,
        std::map<std::string, Tensor>&) override;

private:
    float relu(float) const;
};

#endif
