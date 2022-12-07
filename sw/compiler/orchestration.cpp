#include "orchestration.h"
#include "error.h"
#include "logging.h"

#include <string>
#include <fstream>
#include <iomanip>

// Construct orchestrator
Orchestrator::Orchestrator(const OrchestratorParam& param)
: m_param(param),
  m_matCoreStatus(param.matCoreCount),
  m_vecCoreStatus(param.vecCoreCount),
  m_matrixHandleCount(0)
{
    for (size_t coreID = 0;coreID < m_param.matCoreCount;coreID++) {
        for (size_t regID = 0;regID < m_param.matCacheSize;regID++) {
            m_matCoreStatus[coreID].freeRegIdx.insert(regID);
        }
    }
    for (size_t coreID = 0;coreID < m_param.vecCoreCount;coreID++) {
        for (size_t regID = 0;regID < m_param.vecCacheSize;regID++) {
            m_vecCoreStatus[coreID].freeRegIdx.insert(regID);
        }
    }
}


// Get absolute MatCore/VecCore ID
size_t Orchestrator::getMatCoreID(size_t id) const {
    return id;
}
size_t Orchestrator::getVecCoreID(size_t id) const {
    return m_param.matCoreCount + id;
}

// Save generated program to files
void Orchestrator::compile() {
    // Add halt instruction
    for (size_t id = 0;id < m_param.matCoreCount;id++) {
        MatCoreInst matInst;
        matInst.opcode = MatCoreInstDefn::HALT;
        m_matCoreStatus[id].prog.append(matInst);
    }
    for (size_t id = 0;id < m_param.vecCoreCount;id++) {
        VecCoreInst vecInst;
        vecInst.opcode = VecCoreInstDefn::HALT;
        m_vecCoreStatus[id].prog.append(vecInst);
    }

    // Save to files
    for (size_t id = 0;id < m_param.matCoreCount;id++) {
        std::string filename = "inst_mem" + std::to_string(getMatCoreID(id)) + ".txt";
        SaveProgram(m_matCoreStatus[id].prog.toBinary(), filename);
    }
    for (size_t id = 0;id < m_param.vecCoreCount;id++) {
        std::string filename = "inst_mem" + std::to_string(getVecCoreID(id)) + ".txt";
        SaveProgram(m_vecCoreStatus[id].prog.toBinary(), filename);
    }

    // Save data memory
    for (size_t id = 0;id < m_param.matCoreCount;id++) {
        std::string filename = "data_mem" + std::to_string(getMatCoreID(id)) + ".txt";
        std::ofstream dm(filename);
        dm << std::setprecision(m_param.floatPrecision) << std::fixed;
        for (const auto& elem : m_matCoreStatus[id].dataMem) {
            dm << elem << std::endl;
        }
        dm.close();
    }
    for (size_t id = 0;id < m_param.vecCoreCount;id++) {
        std::string filename = "data_mem" + std::to_string(getVecCoreID(id)) + ".txt";
        std::ofstream dm(filename);
        dm << std::setprecision(m_param.floatPrecision) << std::fixed;
        for (const auto& elem : m_vecCoreStatus[id].dataMem) {
            dm << elem << std::endl;
        }
        dm.close();
    }
}


// Initialize MatrixState
Orchestrator::MatrixState::MatrixState(const MatrixShape& matrixShape)
: matrixShape(matrixShape)
{
    activeRegIdx.assign(matrixShape.x,
        std::vector<size_t>(matrixShape.y, 0));
}


// Data operations
// TODO: The simpler version allocates everything to matcore 0
// In this case veccore are unused by orchestrator

// Allocate a matrix
Orchestrator::MatrixHandle Orchestrator::dataMatrixAllocate(const MatrixShape& matrixShape) {
    MatrixHandle handle = m_matrixHandleCount++;

    // Determine register size
    auto res = m_dataMatrixStatus.emplace(handle,
        MatrixState(matrixShape));
    if (!res.second) {
        FatalError("Duplicate MatrixHandle");
    }

    // Initialize matrixState
    auto& matrixState = res.first->second;
    matrixState.coreIdx = 0;

    for (size_t i = 0;i < matrixShape.x;i++) {
        for (size_t j = 0;j < matrixShape.y;j++) {
            // Attempt to get a free register
            auto freeRegIter = m_matCoreStatus[0].freeRegIdx.begin();
            if (freeRegIter == m_matCoreStatus[0].freeRegIdx.end()) {
                FatalError("Orchestrator alloc no reg");
            }
            size_t regIdx = *freeRegIter;
            m_matCoreStatus[0].freeRegIdx.erase(freeRegIter);

            matrixState.activeRegIdx[i][j] = regIdx;
        }
    }

    // Log allocated registers
    LogInfo("Orchestrator alloc - core 0: " + std::to_string(m_matCoreStatus[0].freeRegIdx.size()) + " remaining");
    LogInfo("    handle: " + std::to_string(handle));
    for (size_t i = 0;i < matrixShape.x;i++) {
        std::string line = "    ";
        for (size_t j = 0;j < matrixShape.y;j++) {
            line += std::to_string(matrixState.activeRegIdx[i][j]) + ",";
        }
        LogInfo(line);
    }

    // Return handle
    return handle;
}

// Deallocate a matrix
void Orchestrator::dataMatrixDeallocate(MatrixHandle handle) {
    // Release registers
    if (m_dataMatrixStatus.count(handle) == 0) {
        FatalError("Orchestrator dealloc " + std::to_string(handle) + " not exist");
    }
    auto& matrixState = m_dataMatrixStatus.at(handle);
    for (size_t i = 0;i < matrixState.matrixShape.x;i++) {
        for (size_t j = 0;j < matrixState.matrixShape.y;j++) {
            size_t regIdx = matrixState.activeRegIdx[i][j];
            m_matCoreStatus[matrixState.coreIdx].freeRegIdx.insert(regIdx);
        }
    }
    m_dataMatrixStatus.erase(handle);

    LogInfo("Orchestrator dealloc - core 0: " + std::to_string(m_matCoreStatus[0].freeRegIdx.size()) + " remaining");
}

// Load matrix from constants
void Orchestrator::dataMatrixLoadConstant(MatrixHandle handle, const MatrixConstant& inputData) {
    const auto& matrixState = m_dataMatrixStatus.at(handle);
    // Store into coreIdx
    auto& dataMem = m_matCoreStatus[matrixState.coreIdx].dataMem;
    auto& prog = m_matCoreStatus[matrixState.coreIdx].prog;

    // Load mat
    for (size_t bx = 0;bx < matrixState.matrixShape.x;bx++) {
        for (size_t by = 0;by < matrixState.matrixShape.y;by++) {
            // Add load instruction
            MatCoreInst inst;
            inst.opcode = MatCoreInstDefn::LOAD_MAT;
            inst.operands[MatCoreInstDefn::ADDR] = dataMem.size(); // Start address
            inst.operands[MatCoreInstDefn::M1] = matrixState.coreIdx;
            prog.append(inst);

            // Add constant to dataMem
            if (inputData[bx][by].size() != m_param.width * m_param.width) {
                FatalError("Orchestrator load constant width mismatch");
            }
            dataMem.insert(dataMem.end(), inputData[bx][by].begin(), inputData[bx][by].end());
        }
    }
}

// Store matrix result to data memory
Orchestrator::MatrixResult Orchestrator::dataMatrixStoreResult(MatrixHandle handle) {
    const auto& matrixState = m_dataMatrixStatus.at(handle);
    auto& dataMem = m_matCoreStatus[matrixState.coreIdx].dataMem;
    auto& prog = m_matCoreStatus[matrixState.coreIdx].prog;

    // Save to dataMem
    std::vector<std::vector<size_t>> dataMemAddr(matrixState.matrixShape.x,
        std::vector<size_t>(matrixState.matrixShape.y, 0));

    for (size_t bx = 0;bx < matrixState.matrixShape.x;bx++) {
        for (size_t by = 0;by < matrixState.matrixShape.y;by++) {
            // Record address
            dataMemAddr[bx][by] = dataMem.size();

            // Add save instruction
            MatCoreInst inst;
            inst.opcode = MatCoreInstDefn::STORE_MAT;
            inst.operands[MatCoreInstDefn::ADDR] = dataMemAddr[bx][by];
            inst.operands[MatCoreInstDefn::M1] = matrixState.coreIdx;
            prog.append(inst);

            // Allocate free space in dataMem
            dataMem.insert(dataMem.end(), m_param.width * m_param.width, 0.0f);
        }
    }
    return MatrixResult{
        .matrixShape = matrixState.matrixShape,
        .coreIdx = matrixState.coreIdx,
        .dataMemAddr = dataMemAddr
    };
}


// Arithmetic operations

// MatMult
Orchestrator::MatrixHandle Orchestrator::arithmeticMatMult(MatrixHandle h1, MatrixHandle h2) {
    // Find h1/h2
    if (m_dataMatrixStatus.count(h1) == 0 ||
        m_dataMatrixStatus.count(h2) == 0) {
        FatalError("Orchestrator matmult handle not exist");
    }
    const auto& m1State = m_dataMatrixStatus.at(h1),
                m2State = m_dataMatrixStatus.at(h2);

    if (m1State.matrixShape.y != m2State.matrixShape.x) {
        FatalError("Orchestrator matmult dim mismatch");
    }

    // Allocate a new matrix
    MatrixHandle hd = dataMatrixAllocate(MatrixShape{
        .x = m1State.matrixShape.x,
        .y = m2State.matrixShape.y
    });
    return hd;
}
