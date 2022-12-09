#ifndef __PRIMITIVE_H__
#define __PRIMITIVE_H__

#include "orchestration.h"

// Generic primitive 
class MatPrimitiveUnary {
public:
    virtual void compile(
        const Orchestrator::ProcState&,
        const Orchestrator::MatrixState&,
        const Orchestrator::MatrixState&
    ) = 0;
};
class MatPrimitiveBinary {
public:
    virtual void compile(
        const Orchestrator::ProcState&,
        const Orchestrator::MatrixState&,
        const Orchestrator::MatrixState&,
        const Orchestrator::MatrixState&
    ) = 0;
};

#endif
