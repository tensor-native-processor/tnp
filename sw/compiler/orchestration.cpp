#include "orchestration.h"
#include "error.h"
#include "logging.h"

#include <string>
#include <fstream>
#include <iomanip>

// MatrixShape comparison
bool operator==(const Orchestrator::MatrixShape& l, const Orchestrator::MatrixShape& r) {
    return l.x == r.x && l.y == r.y;
}
bool operator!=(const Orchestrator::MatrixShape& l, const Orchestrator::MatrixShape& r) {
    return !(l == r);
}

// Construct orchestrator
Orchestrator::Orchestrator(const OrchestratorParam& param)
: m_param(param),
  m_matCoreStatus(param.matCoreCount),
  m_vecCoreStatus(param.vecCoreCount),
  m_matrixHandleCount(0)
{
    for (size_t coreID = 0;coreID < m_param.matCoreCount;coreID++) {
        for (size_t regID = 0;regID < m_param.matCacheSize;regID++) {
            m_matCoreStatus[coreID].m_freeRegIdx.insert(regID);
        }
    }
    for (size_t coreID = 0;coreID < m_param.vecCoreCount;coreID++) {
        for (size_t regID = 0;regID < m_param.vecCacheSize;regID++) {
            m_vecCoreStatus[coreID].m_freeRegIdx.insert(regID);
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
        m_matCoreStatus[id].m_prog.append(matInst);
    }
    for (size_t id = 0;id < m_param.vecCoreCount;id++) {
        VecCoreInst vecInst;
        vecInst.opcode = VecCoreInstDefn::HALT;
        m_vecCoreStatus[id].m_prog.append(vecInst);
    }

    // Save to files
    for (size_t id = 0;id < m_param.matCoreCount;id++) {
        std::string filename = "inst_mem" + std::to_string(getMatCoreID(id)) + ".txt";
        SaveProgram(m_matCoreStatus[id].m_prog.toBinary(), filename);
    }
    for (size_t id = 0;id < m_param.vecCoreCount;id++) {
        std::string filename = "inst_mem" + std::to_string(getVecCoreID(id)) + ".txt";
        SaveProgram(m_vecCoreStatus[id].m_prog.toBinary(), filename);
    }

    // Save data memory
    for (size_t id = 0;id < m_param.matCoreCount;id++) {
        std::string filename = "data_mem" + std::to_string(getMatCoreID(id)) + ".txt";
        std::ofstream dm(filename);
        dm << std::setprecision(m_param.floatPrecision) << std::fixed;
        for (const auto& elem : m_matCoreStatus[id].m_dataMem) {
            dm << elem << std::endl;
        }
        dm.close();
    }
    for (size_t id = 0;id < m_param.vecCoreCount;id++) {
        std::string filename = "data_mem" + std::to_string(getVecCoreID(id)) + ".txt";
        std::ofstream dm(filename);
        dm << std::setprecision(m_param.floatPrecision) << std::fixed;
        for (const auto& elem : m_vecCoreStatus[id].m_dataMem) {
            dm << elem << std::endl;
        }
        dm.close();
    }
}


// Initialize MatrixState
Orchestrator::MatrixState::MatrixState(const MatrixShape& shape)
: m_shape(shape)
{
    m_regIdx.assign(shape.x,
        std::vector<size_t>(shape.y, 0));
}


// Data operations
// TODO: The simpler version allocates everything to matcore 0
// In this case veccore are unused by orchestrator

// Allocate a matrix
Orchestrator::MatrixHandle Orchestrator::dataMatrixAllocate(const MatrixShape& shape) {
    MatrixHandle handle = m_matrixHandleCount++;

    // Determine register size
    auto res = m_dataMatrixStatus.emplace(handle,
        MatrixState(shape));
    if (!res.second) {
        FatalError("Duplicate MatrixHandle");
    }

    // Initialize matrixState
    auto& matrixState = res.first->second;
    matrixState.m_coreIdx = 0;

    for (size_t i = 0;i < shape.x;i++) {
        for (size_t j = 0;j < shape.y;j++) {
            // Attempt to get a free register
            auto freeRegIter = m_matCoreStatus[0].m_freeRegIdx.begin();
            if (freeRegIter == m_matCoreStatus[0].m_freeRegIdx.end()) {
                FatalError("Orchestrator alloc no reg");
            }
            size_t regIdx = *freeRegIter;
            m_matCoreStatus[0].m_freeRegIdx.erase(freeRegIter);

            matrixState.m_regIdx[i][j] = regIdx;
        }
    }

    // Log allocated registers
    LogInfo("Orchestrator alloc - core 0: " + std::to_string(m_matCoreStatus[0].m_freeRegIdx.size()) + " remaining");
    LogInfo("    handle: " + std::to_string(handle));
    for (size_t i = 0;i < shape.x;i++) {
        std::string line = "    ";
        for (size_t j = 0;j < shape.y;j++) {
            line += std::to_string(matrixState.m_regIdx[i][j]) + ",";
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
    for (size_t i = 0;i < matrixState.m_shape.x;i++) {
        for (size_t j = 0;j < matrixState.m_shape.y;j++) {
            size_t regIdx = matrixState.m_regIdx[i][j];
            m_matCoreStatus[matrixState.m_coreIdx].m_freeRegIdx.insert(regIdx);
        }
    }
    m_dataMatrixStatus.erase(handle);

    LogInfo("Orchestrator dealloc - core 0: " + std::to_string(m_matCoreStatus[0].m_freeRegIdx.size()) + " remaining");
}

// Load matrix from constants
void Orchestrator::dataMatrixLoadConstant(MatrixHandle handle, const MatrixConstant& inputData) {
    // Retrieve matrix
    const auto& matrixState = m_dataMatrixStatus.at(handle);
    auto& coreState = m_matCoreStatus[matrixState.m_coreIdx];

    // Validate shape
    if (matrixState.m_shape != inputData.m_shape) {
        FatalError("Orchestrator load constant dim mismatch");
    }

    // Load mat
    for (size_t bx = 0;bx < matrixState.m_shape.x;bx++) {
        for (size_t by = 0;by < matrixState.m_shape.y;by++) {
            // Add load instruction
            MatCoreInst inst;
            inst.opcode = MatCoreInstDefn::LOAD_MAT;
            inst.operands[MatCoreInstDefn::ADDR] = coreState.m_dataMem.size(); // Start address
            inst.operands[MatCoreInstDefn::M1] = matrixState.m_coreIdx;
            coreState.m_prog.append(inst);

            // Add constant to dataMem
            if (inputData.m_data[bx][by].size() != m_param.width * m_param.width) {
                FatalError("Orchestrator load constant width mismatch");
            }
            coreState.m_dataMem.insert(coreState.m_dataMem.end(), inputData.m_data[bx][by].begin(), inputData.m_data[bx][by].end());
        }
    }
}

// Store matrix result to data memory
Orchestrator::MatrixResult Orchestrator::dataMatrixStoreResult(MatrixHandle handle) {
    const auto& matrixState = m_dataMatrixStatus.at(handle);
    auto& coreState = m_matCoreStatus[matrixState.m_coreIdx];

    // Save to dataMem
    std::vector<std::vector<size_t>> dataAddr(matrixState.m_shape.x,
        std::vector<size_t>(matrixState.m_shape.y, 0));

    for (size_t bx = 0;bx < matrixState.m_shape.x;bx++) {
        for (size_t by = 0;by < matrixState.m_shape.y;by++) {
            // Record address
            dataAddr[bx][by] = coreState.m_dataMem.size();

            // Add save instruction
            MatCoreInst inst;
            inst.opcode = MatCoreInstDefn::STORE_MAT;
            inst.operands[MatCoreInstDefn::ADDR] = dataAddr[bx][by];
            inst.operands[MatCoreInstDefn::M1] = matrixState.m_coreIdx;
            coreState.m_prog.append(inst);

            // Allocate free space in dataMem
            coreState.m_dataMem.insert(coreState.m_dataMem.end(), m_param.width * m_param.width, 0.0f);
        }
    }
    return MatrixResult{
        .m_shape = matrixState.m_shape,
        .m_coreIdx = matrixState.m_coreIdx,
        .m_dataAddr = dataAddr
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

    if (m1State.m_shape.y != m2State.m_shape.x) {
        FatalError("Orchestrator matmult dim mismatch");
    }

    // Allocate a new matrix
    auto hd = dataMatrixAllocate(MatrixShape{
        .x = m1State.m_shape.x,
        .y = m2State.m_shape.y
    });
    return hd;
}

// Relu
Orchestrator::MatrixHandle Orchestrator::arithmeticRelu(MatrixHandle handleIn) {
    // Find handle
    if (m_dataMatrixStatus.count(handleIn) == 0) {
        FatalError("Orchestrator relu handle not exist");
    }
    const auto& matrixState = m_dataMatrixStatus.at(handleIn);

    // Allocate a new matrix
    auto handleOut = dataMatrixAllocate(matrixState.m_shape);
    return handleOut;
}

// Transpose
Orchestrator::MatrixHandle Orchestrator::arithmeticTranspose(MatrixHandle handle) {
    // Find handle
    if (m_dataMatrixStatus.count(handle) == 0) {
        FatalError("Orchestrator relu handle not exist");
    }
    const auto& matrixState = m_dataMatrixStatus.at(handle);

    // Allocate a new matrix
    auto handleOut = dataMatrixAllocate({
        matrixState.m_shape.y, matrixState.m_shape.x
    });
    return handleOut;
}
