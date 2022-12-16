#include "orchestration.h"
#include "logging.h"
#include "error.h"

// Relu
Orchestrator::MatrixHandle Orchestrator::arithmeticRelu(MatrixHandle handleIn) {
    return arithmeticReluSingleCore(handleIn);
}

// Single core implementation
Orchestrator::MatrixHandle Orchestrator::arithmeticReluSingleCore(MatrixHandle handleIn) {
    // Find handle
    if (m_dataMatrixState.count(handleIn) == 0) {
        FatalError("Orchestrator relu handle not exist");
    }
    const auto& inState = m_dataMatrixState.at(handleIn);

    // Allocate a new matrix
    auto handleOut = dataMatrixAllocate(inState.m_shape);
    const auto& outState = m_dataMatrixState.at(handleOut);

    // Update cycle count
    simulateCycleCount();

    // Find a free VecCore (at least 1 register)
    // TODO: use a better algorithm
    bool foundFreeVecCore = false;
    size_t vecCoreIdx = 0;
    for (size_t i = 0;i < m_param.vecCoreCount;i++) {
        if (m_procState.vecCores[i].m_freeRegIdx.size() >= 1) {
            foundFreeVecCore = true;
            vecCoreIdx = i;
            break;
        }
    }
    if (!foundFreeVecCore) {
        FatalError("Orchestrator relu no free VecCore");
    }
    auto& vecCore = m_procState.vecCores[vecCoreIdx];
    auto& inCore = m_procState.matCores[inState.m_coreIdx];
    auto& outCore = m_procState.matCores[outState.m_coreIdx];

    // 0 - Retrieve free register from vecCore
    if (vecCore.m_freeRegIdx.empty()) {
        FatalError("Orchestrator relu freeRegIdx should not be empty");
    }
    size_t vecCoreTmpReg = vecCore.m_freeRegIdx.back();
    vecCore.m_freeRegIdx.pop_back();

    // For each matrix register in inState
    for (size_t bx = 0;bx < inState.m_shape.x;bx++) {
        for (size_t by = 0;by < inState.m_shape.y;by++) {
            for (size_t i = 0;i < m_param.width;i++) {
                // 1 - Send inCore -> vecCore
                inCore.m_prog.append({MatCoreInstDefn::SEND_ROW, {
                    {MatCoreInstDefn::CORE_IDX, getVecCoreID(vecCoreIdx)},
                    {MatCoreInstDefn::M1, inState.m_regIdx[bx][by]},
                    {MatCoreInstDefn::ROW_IDX, i}
                }});
                vecCore.m_prog.append({VecCoreInstDefn::RECV_VEC, {
                    {VecCoreInstDefn::CORE_IDX, getMatCoreID(inState.m_coreIdx)},
                    {VecCoreInstDefn::V1, vecCoreTmpReg}
                }});

                // 2 - Relu
                vecCore.m_prog.append({VecCoreInstDefn::ACT_RELU, {
                    {VecCoreInstDefn::Vd, vecCoreTmpReg},
                    {VecCoreInstDefn::V1, vecCoreTmpReg}
                }});

                // 3 - Send vecCore -> outCore
                vecCore.m_prog.append({VecCoreInstDefn::SEND_VEC, {
                    {VecCoreInstDefn::CORE_IDX, getMatCoreID(outState.m_coreIdx)},
                    {VecCoreInstDefn::V1, vecCoreTmpReg}

                }});
                outCore.m_prog.append({MatCoreInstDefn::RECV_ROW, {
                    {MatCoreInstDefn::CORE_IDX, getVecCoreID(vecCoreIdx)},
                    {MatCoreInstDefn::M1, outState.m_regIdx[bx][by]},
                    {MatCoreInstDefn::ROW_IDX, i}
                }});
            }
        }
    }

    // Release vecCoreTmpReg
    vecCore.m_freeRegIdx.push_back(vecCoreTmpReg);

    // Done
    return handleOut;
}
