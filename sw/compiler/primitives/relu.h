#ifndef __PRIMITIVE_RELU_H__
#define __PRIMITIVE_RELU_H__

#include "primitive.h"

// Relu primitive on matrix
class MatPrimitiveRelu : public MatPrimitiveUnary {
public:
    void compile(
        const Orchestrator::ProcState&,
        const Orchestrator::MatrixState&,
        const Orchestrator::MatrixState&
    ) override;
};

#endif
