#include "error.h"
#include "mat_compiler.h"
#include "compiler_common.h" 
#include "orchestration.h"


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
    auto& m1Core = m_procState.matCores[m1State.m_coreIdx];
    // In2
    auto& m2Core = m_procState.matCores[m2State.m_coreIdx];
    // Out
    auto& m3Core = m_procState.matCores[m3State.m_coreIdx];

    // Divide registers among cores (dividing the matrix among cores)
    auto [coresForRows, coresForCols] = getCoreAssignment(m1State.m_shape.x, m1State.m_shape.y);        
    
    auto m1SubRegs = getSubRegs(m1State.m_regIdx, coresForRows, coresForCols);
    auto m2SubRegs = getSubRegs(m2State.m_regIdx, coresForRows, coresForCols);
    auto m3SubRegs = getSubRegs(m3State.m_regIdx, coresForRows, coresForCols);

    // Distribute In1 and In2 to all cores
    // TODO what if a core does not have enough registers to receive??
    for (int i = 0; i < coresForRows; i++) {
        for (int j = 0; j < coresForCols; j++) {
            int matCoreOffset = i * coresForCols + j;
            int matCoreIdx = MAT_CORE_START_IDX + matCoreOffset;

            // // Copy if same core
            // if (m1State.m_coreIdx == matCoreIdx) {
            //     m1Core.m_prog.append({MatCoreInstDefn::COPY, {
            //         {MatCoreInstDefn::M1, m1State.m_regIdx},
            //         {MatCoreInstDefn::Md, 
            //     }});
            // } else {
            //     // Send/Recv
            // }

        }
    }    
    
    MatCoreProgram matProgs[NUM_MAT_CORES];
    VecCoreProgram vecProgs[NUM_VEC_CORES]; 
    std::vector<MatInfo> subMatInfos1;
    std::vector<MatInfo> subMatInfos2;
    // Perform Matmult
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
    for (int i = 0; i < coresForRows; i++) {
        for (int j = 0; j < coresForCols; j++) {
            int matCoreOffset = i * coresForCols + j;
            int vecCoreOffset = i * coresForCols + j;
            int matCoreIdx = MAT_CORE_START_IDX + matCoreOffset;
            int vecCoreIdx = VEC_CORE_START_IDX + vecCoreOffset;
            
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
                MatInfo(m1subReg, m2subReg, m3subReg, 
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
                    m3subReg = m3SubRegs[0][0];
                    break;
                }
            }

            subMatInfos2.push_back(
                MatInfo(m1subReg, m2subReg, m3subReg,
                m_procState.matCores[matCoreIdx].m_freeRegIdx,
                m_procState.matCores[matCoreIdx].m_dataMem.size()) 
            );

            subMatInfos2[matCoreIdx].matAMemStart = 0;
            subMatInfos2[matCoreIdx].matBMemStart = subMatInfos1[matCoreIdx].matAMemSize + 
                subMatInfos1[matCoreIdx].matBMemSize + subMatInfos1[matCoreIdx].matCMemSize;
            subMatInfos2[matCoreIdx].matCMemStart = 
                subMatInfos2[matCoreIdx].matBMemStart + subMatInfos2[matCoreIdx].matBMemSize;
        }
    }
    
    // m_procState.matCores[0].m_prog

    // std::vector<std::tuple<int, int, int>> addCoreIdxs1{{0, 1, 0}, {2, 3, 3}};
    // multiMultAndAdd(coresForRows, coresForCols, subMatInfos1, 
    // matProgs, vecProgs, addCoreIdxs1);
    
    // std::vector<std::tuple<int, int, int>> addCoreIdxs2{{0, 1, 1}, {2, 3, 2}};
    // multiMultAndAdd(coresForRows, coresForCols, subMatInfos2, 
    // matProgs, vecProgs, addCoreIdxs2);
    
    // append progs to original prog

    // Gather result from all cores to Out

    return h3;
}