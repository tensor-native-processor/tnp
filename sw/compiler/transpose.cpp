#include "orchestration.h"
#include "logging.h"
#include "error.h"

// Copy
Orchestrator::MatrixHandle Orchestrator::arithmeticCopy(MatrixHandle handleIn) {
    // Find handle
    if (m_dataMatrixState.count(handleIn) == 0) {
        FatalError("Orchestrator copy handle not exist");
    }

    // Allocate a new matrix
    const auto& inState = m_dataMatrixState.at(handleIn);
    auto handleOut = dataMatrixAllocate({
        inState.m_shape.y, inState.m_shape.x
    });
    const auto& outState = m_dataMatrixState.at(handleOut);

    auto& inCore = m_procState.matCores[inState.m_coreIdx];
    auto& outCore = m_procState.matCores[outState.m_coreIdx];

    // Copy from handleIn to handleOut
    for (size_t bx = 0;bx < outState.m_shape.x;bx++) {
        for (size_t by = 0;by < outState.m_shape.y;by++) {
            if (inState.m_coreIdx == outState.m_coreIdx) {
                // Copy matrix (same core)
                inCore.m_prog.append({MatCoreInstDefn::COPY, {
                    {MatCoreInstDefn::M1, inState.m_regIdx[by][bx]},
                    {MatCoreInstDefn::Md, outState.m_regIdx[bx][by]}
                }});
            } else {
                // Send/Recv matrix
                for (size_t r = 0;r < m_param.width;r++) {
                    // Send
                    inCore.m_prog.append({MatCoreInstDefn::SEND_ROW, {
                        {MatCoreInstDefn::CORE_IDX, getMatCoreID(outState.m_coreIdx)},
                        {MatCoreInstDefn::M1, inState.m_regIdx[by][bx]},
                        {MatCoreInstDefn::ROW_IDX, r}
                    }});
                    // Recv
                    outCore.m_prog.append({MatCoreInstDefn::RECV_ROW, {
                        {MatCoreInstDefn::CORE_IDX, getMatCoreID(inState.m_coreIdx)},
                        {MatCoreInstDefn::M1, outState.m_regIdx[bx][by]},
                        {MatCoreInstDefn::ROW_IDX, r}
                    }});
                }
            }
        }
    }
    return handleOut;
}

// Transpose
Orchestrator::MatrixHandle Orchestrator::arithmeticTranspose(MatrixHandle handleIn) {
    auto handleOut = arithmeticCopy(handleIn);
    arithmeticTransposeSelf(handleOut);
    return handleOut;
}

void Orchestrator::arithmeticTransposeSelf(MatrixHandle handleIn) {
    // Find handle
    if (m_dataMatrixState.count(handleIn) == 0) {
        FatalError("Orchestrator transpose handle not exist");
    }

    // Find MatrixState for handleIn
    const auto& inState = m_dataMatrixState.at(handleIn);
    auto& inCore = m_procState.matCores[inState.m_coreIdx];

    // Transpose
    for (size_t bx = 0;bx < inState.m_shape.x;bx++) {
        for (size_t by = 0;by < inState.m_shape.y;by++) {
            inCore.m_prog.append({MatCoreInstDefn::TRANSPOSE, {
                {MatCoreInstDefn::M1, inState.m_regIdx[bx][by]}
            }});
        }
    }
}
