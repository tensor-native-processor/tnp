#ifndef __OPERATOR_RELU_H__
#define __OPERATOR_RELU_H__

#include "operator.h"

// Relu
class OperatorRelu: public Operator {
    void inferShape(const ::onnx::NodeProto&,
        const std::map<std::string, Tensor>&,
        std::map<std::string, Shape>&) const override;

    void simulate(const ::onnx::NodeProto&,
        std::map<std::string, Tensor>&) const override;
};

#endif
