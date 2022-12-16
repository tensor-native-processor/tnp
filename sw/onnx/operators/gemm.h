#ifndef __OPERATOR_GEMM_H__
#define __OPERATOR_GEMM_H__

#include "operator.h"

// Gemm
class OperatorGemm: public Operator {
public:
    void inferShape(const ::onnx::NodeProto&,
        const std::map<std::string, Tensor>&,
        std::map<std::string, Tensor::Shape>&) override;

    void simulate(const ::onnx::NodeProto&,
        std::map<std::string, Tensor>&) override;

    void compile(const ::onnx::NodeProto&,
        Orchestrator&,
        std::map<std::string, Orchestrator::MatrixHandle>&,
        std::map<std::string, size_t>&
    ) override;

private:
    void getAttributes(const ::onnx::NodeProto&);
    bool attr_transA, attr_transB;
    float attr_alpha, attr_beta;
};

#endif
