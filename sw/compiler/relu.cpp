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
    auto vecCoreTmpIter = vecCore.m_freeRegIdx.begin();
    size_t vecCoreTmpReg = *vecCoreTmpIter;
    vecCore.m_freeRegIdx.erase(vecCoreTmpIter);

    // For each matrix register in inState
    for (size_t bx = 0;bx < inState.m_shape.x;bx++) {
        for (size_t by = 0;by < inState.m_shape.y;by++) {
            for (size_t i = 0;i < m_param.width;i++) {
                MatCoreInst matInst;
                VecCoreInst vecInst;

                // 1 - Send inCore -> vecCore
                matInst.opcode = MatCoreInstDefn::SEND_ROW;
                matInst.operands[MatCoreInstDefn::CORE_IDX] = getVecCoreID(vecCoreIdx);
                matInst.operands[MatCoreInstDefn::M1] = inState.m_regIdx[bx][by];
                matInst.operands[MatCoreInstDefn::ROW_IDX] = i;
                inCore.m_prog.append(matInst);

                vecInst.opcode = VecCoreInstDefn::RECV_VEC;
                vecInst.operands[VecCoreInstDefn::CORE_IDX] = getMatCoreID(inState.m_coreIdx);
                vecInst.operands[VecCoreInstDefn::V1] = vecCoreTmpReg;
                vecCore.m_prog.append(vecInst);

                // 2 - Relu
                vecInst.opcode = VecCoreInstDefn::ACT_RELU;
                vecInst.operands[VecCoreInstDefn::Vd] = vecCoreTmpReg;
                vecInst.operands[VecCoreInstDefn::V1] = vecCoreTmpReg;
                vecCore.m_prog.append(vecInst);

                // 3 - Send vecCore -> outCore
                vecInst.opcode = VecCoreInstDefn::SEND_VEC;
                vecInst.operands[VecCoreInstDefn::CORE_IDX] = getMatCoreID(outState.m_coreIdx);
                vecInst.operands[VecCoreInstDefn::V1] = vecCoreTmpReg;
                vecCore.m_prog.append(vecInst);

                matInst.opcode = MatCoreInstDefn::RECV_ROW;
                matInst.operands[MatCoreInstDefn::CORE_IDX] = getVecCoreID(vecCoreIdx);
                matInst.operands[MatCoreInstDefn::M1] = outState.m_regIdx[bx][by];
                matInst.operands[MatCoreInstDefn::ROW_IDX] = i;
                outCore.m_prog.append(matInst);
            }
        }
    }

    // Release vecCoreTmpReg
    vecCore.m_freeRegIdx.insert(vecCoreTmpReg);

    // Done
    return handleOut;
}
