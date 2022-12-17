#include "error.h"
#include "mat_compiler.h"
#include "compiler_common.h" 
#include "orchestration.h"

// Copied from getSubMats() TODO: dedup
std::vector<std::vector<std::vector<std::vector<size_t>>>> 
    getSubRegs(const std::vector<std::vector<size_t>> &mat, int coresForRows, int coresForCols) {
    
    std::vector<std::vector<std::vector<std::vector<size_t>>>> subMats;  
    int subMatRows = (mat.size() + coresForRows - 1) / coresForRows;
    int subMatCols = (mat[0].size() + coresForCols - 1) / coresForCols;
     
    for (int rCoreIdx = 0; rCoreIdx < coresForRows; rCoreIdx++) {
        std::vector<std::vector<std::vector<size_t>>> subMatsPerRow;
        int rStart = rCoreIdx * subMatRows;
        int rEnd = std::min((rCoreIdx + 1) * subMatRows, static_cast<int>(mat.size()));
        
        for (int cCoreIdx = 0; cCoreIdx < coresForCols; cCoreIdx++) {
            std::vector<std::vector<size_t>> subMat;
            int cStart = cCoreIdx * subMatCols;
            int cEnd = std::min((cCoreIdx + 1) * subMatCols, static_cast<int>(mat[0].size())); 
            
            for (int i = rStart; i < rEnd; i++) {
                std::vector<size_t> row(mat[i].begin() + cStart, mat[i].begin() + cEnd);
                subMat.push_back(row);
            }
            subMatsPerRow.push_back(subMat);
        }
        subMats.push_back(subMatsPerRow);
    }

    return subMats;
}

void distributeMatViaReg(
    const int sendCoreIdx,
    const int recvCoreIdx,
    MatCoreProgram &sendProg,
    MatCoreProgram &recvProg,
    const std::vector<std::vector<size_t>> matReg,
    const int matRegStart,
    const int matMaxRegs,
    const int matMemStart,
    int (&matRegToMemAddr)[MAT_REG_SIZE],
    const std::vector<size_t> &regMap
    ) {
    for (size_t bx = 0;bx < matReg.size();bx++) {
        for (size_t by = 0;by < matReg[0].size();by++) {
            int regOffset = bx * matReg.size() + by;
            int recvReg = regMap[matRegStart + regOffset % matMaxRegs];

            if (sendCoreIdx == recvCoreIdx) {
                // Copy matrix (same core)
                sendProg.append({MatCoreInstDefn::COPY, {
                    {MatCoreInstDefn::M1, matReg[bx][by]},
                    {MatCoreInstDefn::Md, recvReg}
                }});
            } else {
                // Send/Recv matrix
                for (size_t r = 0;r < BLOCK_WIDTH;r++) {
                    // Send
                    sendProg.append({MatCoreInstDefn::SEND_ROW, {
                        {MatCoreInstDefn::CORE_IDX, recvCoreIdx},
                        {MatCoreInstDefn::M1, matReg[bx][by]},
                        {MatCoreInstDefn::ROW_IDX, r}
                    }});
                    // Recv
                    recvProg.append({MatCoreInstDefn::RECV_ROW, {
                        {MatCoreInstDefn::CORE_IDX, sendCoreIdx},
                        {MatCoreInstDefn::M1, recvReg},
                        {MatCoreInstDefn::ROW_IDX, r}
                    }});
                }
            }
        }
    }
}

void gatherMatViaReg(
    const int sendCoreIdx,
    const int recvCoreIdx,
    MatCoreProgram &sendProg,
    MatCoreProgram &recvProg,
    const std::vector<std::vector<size_t>> recvMatReg,
    const int matRegStart,
    const int matMaxRegs,
    const int matMemStart,
    int (&matRegToMemAddr)[MAT_REG_SIZE],
    const std::vector<size_t> &regMap
    ) {
    for (size_t bx = 0;bx < recvMatReg.size();bx++) {
        for (size_t by = 0;by < recvMatReg[0].size();by++) {
            int sendRegOffset = bx * recvMatReg.size() + by;
            int sendReg = regMap[matRegStart + sendRegOffset % matMaxRegs];
            // int sendAddr = matMemStart + sendRegOffset * BLOCK_AREA;

            if (sendCoreIdx == recvCoreIdx) {
                sendProg.append({MatCoreInstDefn::COPY, {
                    {MatCoreInstDefn::M1, sendReg},
                    {MatCoreInstDefn::Md, recvMatReg[bx][by]}
                }});
            } else {
                for (size_t r = 0;r < BLOCK_WIDTH;r++) {
                    // Send
                    sendProg.append({MatCoreInstDefn::SEND_ROW, {
                        {MatCoreInstDefn::CORE_IDX, recvCoreIdx},
                        {MatCoreInstDefn::M1, sendReg},
                        {MatCoreInstDefn::ROW_IDX, r}
                    }});
                    // Recv
                    recvProg.append({MatCoreInstDefn::RECV_ROW, {
                        {MatCoreInstDefn::CORE_IDX, sendCoreIdx},
                        {MatCoreInstDefn::M1, recvMatReg[bx][by]},
                        {MatCoreInstDefn::ROW_IDX, r}
                    }});
                }
            }
        }
    }
}

void appendProgs(const int coresForRows, const int coresForCols,
    MatCoreProgram (&matProgs)[NUM_MAT_CORES], VecCoreProgram (&vecProgs)[NUM_VEC_CORES],
    Orchestrator::ProcState &procState
) {
    // append progs to original prog
    for (int i = 0; i < coresForRows; i++) {
        for (int j = 0; j < coresForCols; j++) {
            int matCoreOffset = i * coresForCols + j;
            int vecCoreOffset = i * coresForCols + j;
            
            for (auto & inst: matProgs[matCoreOffset].getInsts()) {
                procState.matCores[matCoreOffset].m_prog.append(inst);
            }

            for (auto & inst: vecProgs[vecCoreOffset].getInsts()) {
                procState.vecCores[vecCoreOffset].m_prog.append(inst);
            }
        }
    }
}

Orchestrator::MatrixHandle Orchestrator::arithmeticMatMult(MatrixHandle h1, MatrixHandle h2) {
    // Find h1/h2
    if (m_dataMatrixState.count(h1) == 0 ||
        m_dataMatrixState.count(h2) == 0) {
        FatalError("Orchestrator matmult handle not exist");
    }
    const auto& m1State = m_dataMatrixState.at(h1),
                m2State = m_dataMatrixState.at(h2);

    if (m1State.m_shape.y != m2State.m_shape.x) {
        FatalError("Orchestrator matmult dim mismatch");
    }

    // Allocate a new matrix
    auto h3 = dataMatrixAllocate(MatrixShape{
        .x = m1State.m_shape.x,
        .y = m2State.m_shape.y
    });
    const auto& m3State = m_dataMatrixState.at(h3);

    // In1
    const int in1CoreIdx = m1State.m_coreIdx;
    auto& in1Core = m_procState.matCores[in1CoreIdx];
    // In2
    const int in2CoreIdx = m2State.m_coreIdx;
    auto& in2Core = m_procState.matCores[in2CoreIdx];
    // Out
    const int outCoreIdx = m3State.m_coreIdx;
    auto& outCore = m_procState.matCores[outCoreIdx];

    auto [coresForRows, coresForCols] = getCoreAssignment(m1State.m_shape.x, m1State.m_shape.y);        
    
    // Divide registers among cores (i.e. dividing the matrix among cores)
    auto m1SubRegs = getSubRegs(m1State.m_regIdx, coresForRows, coresForCols);
    auto m2SubRegs = getSubRegs(m2State.m_regIdx, coresForRows, coresForCols);
    auto m3SubRegs = getSubRegs(m3State.m_regIdx, coresForRows, coresForCols);
    
    std::vector<MatInfo> subMatInfos1;
    std::vector<MatInfo> subMatInfos2;
    
    /* MULT & ADD 1 (only support 4 cores for now)
        MULT       ADD
    core  A  B
        0: 00 00 --> vec0 -> mat0  
        1: 01 10 /
        2: 10 01 \
        3: 11 11 --> vec3 -> mat3
    */
    /* MULT & ADD 2 (only support 4 cores for now)
          MULT       ADD
     core  A  B
        0: 00 01 \   
        1: 01 11 --> vec1 -> mat1
        2: 10 00 --> vec2 -> mat2
        3: 11 10 / 
    */
   // Initialization
    for (int i = 0; i < coresForRows; i++) {
        for (int j = 0; j < coresForCols; j++) {
            int matCoreOffset = i * coresForCols + j;
            int matCoreIdx = MAT_CORE_START_IDX + matCoreOffset;
            
            std::vector<std::vector<size_t>> m1subReg = m1SubRegs[i][j];
            std::vector<std::vector<size_t>> m2subReg; 
            std::vector<std::vector<size_t>> m3subReg;

            // MULT & ADD 1 data (HARD CODED)
            if (matCoreOffset == 1 || matCoreOffset == 2) {
                m2subReg = m2SubRegs[j][i];
                m3subReg = m3SubRegs[i][i];
            } else {
                m2subReg = m2SubRegs[i][j];
                m3subReg = m3SubRegs[i][j];
            }
            subMatInfos1.push_back(
                MatInfo(matCoreIdx, m1subReg, m2subReg, m3subReg, 
                m_procState.matCores[matCoreIdx].m_freeRegIdx,
                m_procState.matCores[matCoreIdx].m_dataMem.size())
            );

            // MULT & ADD 2 data (HARD CODED)
            switch (matCoreIdx) {
                case 0: {
                    m2subReg = m2SubRegs[0][1];
                    m3subReg = m3SubRegs[0][1];
                    break;
                }
                case 1: {
                    m2subReg = m2SubRegs[1][1];
                    m3subReg = m3SubRegs[0][1];
                    break;
                }
                case 2: {
                    m2subReg = m2SubRegs[0][0];
                    m3subReg = m3SubRegs[1][0];
                    break;
                }
                case 3: {
                    m2subReg = m2SubRegs[1][0];
                    m3subReg = m3SubRegs[1][0];
                    break;
                }
            }

            subMatInfos2.push_back(
                MatInfo(matCoreIdx, m1subReg, m2subReg, m3subReg,
                m_procState.matCores[matCoreIdx].m_freeRegIdx,
                m_procState.matCores[matCoreIdx].m_dataMem.size()) 
            );

            // Adjust the memory address for MULT & ADD 2
            subMatInfos2[matCoreIdx].matAMemStart = 0;
            subMatInfos2[matCoreIdx].matBMemStart = subMatInfos1[matCoreIdx].matAMemSize + 
                subMatInfos1[matCoreIdx].matBMemSize + subMatInfos1[matCoreIdx].matCMemSize;
            subMatInfos2[matCoreIdx].matCMemStart = 
                subMatInfos2[matCoreIdx].matBMemStart + subMatInfos2[matCoreIdx].matBMemSize;
        }
    }

    // Distribute In1 and In2 to all cores
    for (int i = 0; i < coresForRows; i++) {
        for (int j = 0; j < coresForCols; j++) {
            int matCoreOffset = i * coresForCols + j;
            int matCoreIdx = MAT_CORE_START_IDX + matCoreOffset;
            auto &mi1 = subMatInfos1[matCoreOffset];
            auto &matProg = m_procState.matCores[matCoreIdx].m_prog;

            // MULT & ADD 1 - In1 distribution
            distributeMatViaReg(
                in1CoreIdx, matCoreIdx, in1Core.m_prog, matProg,
                mi1.matAReg, mi1.matARegStart, mi1.matAMaxRegs, mi1.matAMemStart,
                mi1.matRegToMemAddr, mi1.regMap
            );

        }
    }    

    for (int i = 0; i < coresForRows; i++) {
        for (int j = 0; j < coresForCols; j++) {
            int matCoreOffset = i * coresForCols + j;
            int matCoreIdx = MAT_CORE_START_IDX + matCoreOffset;
            auto &mi1 = subMatInfos1[matCoreOffset];
            auto &matProg = m_procState.matCores[matCoreIdx].m_prog;

            // MULT & ADD 1 - In2 distribution
            distributeMatViaReg(
                in2CoreIdx, matCoreIdx, in2Core.m_prog, matProg,
                mi1.matBReg, mi1.matBRegStart, mi1.matBMaxRegs, mi1.matBMemStart,
                mi1.matRegToMemAddr, mi1.regMap
            );
        }
    }    

    // MULT & ADD 1 
    MatCoreProgram matProgs[NUM_MAT_CORES];
    VecCoreProgram vecProgs[NUM_VEC_CORES]; 
    std::vector<std::tuple<int, int, int>> addCoreIdxs1{{0, 1, 0}, {2, 3, 3}};
    multiMultAndAdd(coresForRows, coresForCols, subMatInfos1, 
        matProgs, vecProgs, addCoreIdxs1, true);
    appendProgs(coresForRows, coresForCols, matProgs, vecProgs, m_procState);

    // Send result from cores 0 3 to Out
    std::vector<int> cores1 {0, 3};
    for (int i : cores1) {
        int matCoreIdx = MAT_CORE_START_IDX + i;
        auto &mi = subMatInfos1[i];
        gatherMatViaReg(
            matCoreIdx, outCoreIdx, 
            m_procState.matCores[matCoreIdx].m_prog, outCore.m_prog,
            mi.matCReg, mi.matCRegStart, mi.matCMaxRegs, mi.matCMemStart,
            mi.matRegToMemAddr, mi.regMap
        );
    }

    // MULT & ADD 2 - In1 distribution (not needed as we can reuse the previous In1)
    // MULT & ADD 2 - In2 distribution
    for (int i = 0; i < coresForRows; i++) {
        for (int j = 0; j < coresForCols; j++) {
            int matCoreOffset = i * coresForCols + j;
            int matCoreIdx = MAT_CORE_START_IDX + matCoreOffset;
            auto &mi2 = subMatInfos2[matCoreOffset];
            auto &matProg = m_procState.matCores[matCoreIdx].m_prog;

            // MULT & ADD 2 - In2 distribution
            distributeMatViaReg(
                in2CoreIdx, matCoreIdx, in2Core.m_prog, matProg,
                mi2.matBReg, mi2.matBRegStart, mi2.matBMaxRegs, mi2.matBMemStart,
                mi2.matRegToMemAddr, mi2.regMap
            );  
        }
    }

    // MULT & ADD 2 
    MatCoreProgram matProgs2[NUM_MAT_CORES];
    VecCoreProgram vecProgs2[NUM_VEC_CORES]; 
    std::vector<std::tuple<int, int, int>> addCoreIdxs2{{0, 1, 1}, {2, 3, 2}};
    multiMultAndAdd(coresForRows, coresForCols, subMatInfos2, 
        matProgs2, vecProgs2, addCoreIdxs2, false);
    appendProgs(coresForRows, coresForCols, matProgs2, vecProgs2, m_procState);

    // Send result from cores 1 2 to Out
    std::vector<int> cores2 {1, 2};
    for (int i : cores2) {
        int matCoreIdx = MAT_CORE_START_IDX + i;
        auto &mi = subMatInfos2[i];
        gatherMatViaReg(
            matCoreIdx, outCoreIdx, 
            m_procState.matCores[matCoreIdx].m_prog, outCore.m_prog,
            mi.matCReg, mi.matCRegStart, mi.matCMaxRegs, mi.matCMemStart,
            mi.matRegToMemAddr, mi.regMap
        );
    }

    return h3;
}