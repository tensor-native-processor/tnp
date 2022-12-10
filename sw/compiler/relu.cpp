#include "orchestration.h"
#include "logging.h"
#include "error.h"

// Relu
Orchestrator::MatrixHandle Orchestrator::arithmeticRelu(MatrixHandle handleIn) {
    // Find handle
    if (m_dataMatrixState.count(handleIn) == 0) {
        FatalError("Orchestrator relu handle not exist");
    }
    const auto& inState = m_dataMatrixState.at(handleIn);

    // Allocate a new matrix
    auto handleOut = dataMatrixAllocate(inState.m_shape);
    const auto& outState = m_dataMatrixState.at(handleOut);

    return handleOut;
}
