#ifndef __OPERATOR_H__
#define __OPERATOR_H__

#include "model.h"

#include <map>
#include <string>
#include <utility>


// Generic operator
class Operator {
public:
    virtual void inferShape(const ::onnx::NodeProto&,
        const std::map<std::string, Tensor>&,
        std::map<std::string, Shape>&) const;
};


// Gemm
class OperatorGemm: public Operator {
    void inferShape(const ::onnx::NodeProto&,
        const std::map<std::string, Tensor>&,
        std::map<std::string, Shape>&) const override;
};


// Relu
class OperatorRelu: public Operator {
    void inferShape(const ::onnx::NodeProto&,
        const std::map<std::string, Tensor>&,
        std::map<std::string, Shape>&) const override;
};

#endif
