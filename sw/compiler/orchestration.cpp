#include "orchestration.h"
#include "error.h"
#include "logging.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

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
  m_matrixHandleCount(0)
{
    // Processor state
    m_procState.matCores.assign(param.matCoreCount, MatCoreState{});
    m_procState.vecCores.assign(param.vecCoreCount, VecCoreState{});

    // Free registers
    for (size_t coreID = 0;coreID < m_param.matCoreCount;coreID++) {
        for (size_t regID = 0;regID < m_param.matCacheSize;regID++) {
            m_procState.matCores[coreID].m_freeRegIdx.push_back(regID);
        }
    }
    for (size_t coreID = 0;coreID < m_param.vecCoreCount;coreID++) {
        for (size_t regID = 0;regID < m_param.vecCacheSize;regID++) {
            m_procState.vecCores[coreID].m_freeRegIdx.push_back(regID);
        }
    }
}


// MatrixResult to hint
std::string Orchestrator::MatrixResult::toHintLine(size_t width) const {
    std::stringstream ss;
    ss << m_coreIdx;
    for (size_t i = 0;i < m_shape.x * width;i++) {
        for (size_t j = 0;j < m_shape.y * width;j++) {
            ss << "," << m_dataAddr[i / width][j / width] + width * (i % width) + j % width;
        }
    }
    ss << "\n";
    return ss.str();
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
        m_procState.matCores[id].m_prog.append({MatCoreInstDefn::HALT, {
        }});
    }
    for (size_t id = 0;id < m_param.vecCoreCount;id++) {
        m_procState.vecCores[id].m_prog.append({VecCoreInstDefn::HALT, {
        }});
    }

    // Save to files
    for (size_t id = 0;id < m_param.matCoreCount;id++) {
        std::string filename = "inst_mem" + std::to_string(getMatCoreID(id)) + ".txt";
        SaveProgram(m_procState.matCores[id].m_prog.toBinary(), filename);

        std::ofstream matTxt("inst_mem" + std::to_string(getMatCoreID(id)) + "_text.txt");
        matTxt << m_procState.matCores[id].m_prog.toText();
        matTxt.close();
    }
    for (size_t id = 0;id < m_param.vecCoreCount;id++) {
        std::string filename = "inst_mem" + std::to_string(getVecCoreID(id)) + ".txt";
        SaveProgram(m_procState.vecCores[id].m_prog.toBinary(), filename);

        std::ofstream vecTxt("inst_mem" + std::to_string(getVecCoreID(id)) + "_text.txt");
        vecTxt << m_procState.vecCores[id].m_prog.toText();
        vecTxt.close();
    }

    // Save data memory
    for (size_t id = 0;id < m_param.matCoreCount;id++) {
        std::string filename = "data_mem" + std::to_string(getMatCoreID(id)) + ".txt";
        std::ofstream dm(filename);
        dm << std::setprecision(m_param.floatPrecision) << std::fixed;
        for (const auto& elem : m_procState.matCores[id].m_dataMem) {
            dm << elem << std::endl;
        }
        dm.close();
    }
    for (size_t id = 0;id < m_param.vecCoreCount;id++) {
        std::string filename = "data_mem" + std::to_string(getVecCoreID(id)) + ".txt";
        std::ofstream dm(filename);
        dm << std::setprecision(m_param.floatPrecision) << std::fixed;
        for (const auto& elem : m_procState.vecCores[id].m_dataMem) {
            dm << elem << std::endl;
        }
        dm.close();
    }
}


// Initialize MatrixState
Orchestrator::MatrixState::MatrixState(const MatrixShape& shape)
: m_shape(shape), m_coreIdx(0)
{
    m_regIdx.assign(shape.x,
        std::vector<size_t>(shape.y, 0));
}
Orchestrator::MatrixState::MatrixState(const MatrixShape& shape, size_t coreIdx, const std::vector<std::vector<size_t>>& regIdx)
: m_shape(shape), m_coreIdx(coreIdx), m_regIdx(regIdx)
{}


// Finding the best core index for allocation
size_t Orchestrator::dataMatrixAllocateDetermineTargetCoreIdx(const MatrixShape& shape) {
    // Find maximum freeRegIdx
    size_t targetCore = 0;
    size_t targetCoreFreeReg = 0;
    for (size_t i = 0;i < m_param.matCoreCount;i++) {
        if (m_procState.matCores[i].m_freeRegIdx.size() >= targetCoreFreeReg) {
            targetCore = i;
            targetCoreFreeReg = m_procState.matCores[i].m_freeRegIdx.size();
        }
    }

    // Validate if there are enough register for shape
    if (targetCoreFreeReg < shape.x * shape.y) {
        FatalError("Orchestrator alloc all cores insufficient reg");
    }
    return targetCore;
}


// Data operations
// Allocate a matrix
Orchestrator::MatrixHandle Orchestrator::dataMatrixAllocate(const MatrixShape& shape) {
    MatrixHandle handle = m_matrixHandleCount++;

    // Determine register size
    auto res = m_dataMatrixState.emplace(handle,
        MatrixState(shape));
    if (!res.second) {
        FatalError("Duplicate MatrixHandle");
    }
    auto& matrixState = res.first->second;

    // Initialize matrixState
    // Attempt to find a MatCore for the matrix
    matrixState.m_coreIdx = dataMatrixAllocateDetermineTargetCoreIdx(shape);
    auto& matCore = m_procState.matCores[matrixState.m_coreIdx];

    // Fetch first shape.x * shape.y elements from matCore.m_freeRegIdx
    for (size_t i = 0;i < shape.x;i++) {
        for (size_t j = 0;j < shape.y;j++) {
            // Attempt to get one free register
            if (matCore.m_freeRegIdx.empty()) {
                FatalError("Orchestrator alloc freeRegIdx should not be empty");
            }
            matrixState.m_regIdx[i][j] = matCore.m_freeRegIdx.back();
            matCore.m_freeRegIdx.pop_back();
        }
    }

    // Log allocated registers
    LogInfo("Orchestrator alloc (M-" + std::to_string(handle) + ") - core " + std::to_string(matrixState.m_coreIdx) + ": " + std::to_string(matCore.m_freeRegIdx.size()) + " remaining");
    LogInfo("    Shape: " + std::to_string(shape.x) + ", " + std::to_string(shape.y));
    /*
    for (size_t i = 0;i < shape.x;i++) {
        std::string line = "    ";
        for (size_t j = 0;j < shape.y;j++) {
            line += std::to_string(matrixState.m_regIdx[i][j]) + ",";
        }
        LogInfo(line);
    }
    */

    // Return handle
    return handle;
}
// Allocate a matrix (with given registers)
Orchestrator::MatrixHandle Orchestrator::dataMatrixAllocate(
        const MatrixShape& shape,
        size_t coreIdx,
        const std::vector<std::vector<size_t>>& regIdx
) {
    MatrixHandle handle = m_matrixHandleCount++;

    // Insert as MatrixState
    auto res = m_dataMatrixState.emplace(handle,
        MatrixState(shape, coreIdx, regIdx));
    if (!res.second) {
        FatalError("Duplicate MatrixHandle");
    }
    auto& matCore = m_procState.matCores[coreIdx];

    // Fetch first shape.x * shape.y elements from matCore.m_freeRegIdx
    for (size_t i = 0;i < shape.x;i++) {
        for (size_t j = 0;j < shape.y;j++) {
            // Remove m_regIdx[i][j] from freeRegIdx
            auto regIter = std::find(matCore.m_freeRegIdx.begin(), matCore.m_freeRegIdx.end(), regIdx[i][j]);
            if (regIter == matCore.m_freeRegIdx.end()) {
                FatalError("Orchestrator alloc already alloc reg");
            }
            matCore.m_freeRegIdx.erase(regIter);
        }
    }

    // Log allocated registers
    LogInfo("Orchestrator alloc (M-" + std::to_string(handle) + ") - core " + std::to_string(coreIdx) + ": " + std::to_string(matCore.m_freeRegIdx.size()) + " remaining");
    LogInfo("    Shape: " + std::to_string(shape.x) + ", " + std::to_string(shape.y));

    // Return handle
    return handle;
}

// Deallocate a matrix
void Orchestrator::dataMatrixDeallocate(MatrixHandle handle) {
    // Release registers
    if (m_dataMatrixState.count(handle) == 0) {
        FatalError("Orchestrator dealloc " + std::to_string(handle) + " not exist");
    }
    auto& matrixState = m_dataMatrixState.at(handle);
    auto& matCore = m_procState.matCores[matrixState.m_coreIdx];

    for (size_t i = 0;i < matrixState.m_shape.x;i++) {
        for (size_t j = 0;j < matrixState.m_shape.y;j++) {
            size_t regIdx = matrixState.m_regIdx[i][j];
            matCore.m_freeRegIdx.push_back(regIdx);
        }
    }
    m_dataMatrixState.erase(handle);

    LogInfo("Orchestrator dealloc (M-" + std::to_string(handle) + ") - core " + std::to_string(matrixState.m_coreIdx) + ": " + std::to_string(matCore.m_freeRegIdx.size()) + " remaining");
}

// Load matrix from constants
void Orchestrator::dataMatrixLoadConstant(MatrixHandle handle, const MatrixConstant& inputData) {
    // Retrieve matrix
    const auto& matrixState = m_dataMatrixState.at(handle);
    auto& coreState = m_procState.matCores[matrixState.m_coreIdx];

    // Validate shape
    if (matrixState.m_shape != inputData.m_shape) {
        FatalError("Orchestrator load constant dim mismatch");
    }

    // Load mat
    for (size_t bx = 0;bx < matrixState.m_shape.x;bx++) {
        for (size_t by = 0;by < matrixState.m_shape.y;by++) {
            // Add load instruction
            coreState.m_prog.append({MatCoreInstDefn::LOAD_MAT, {
                {MatCoreInstDefn::ADDR, coreState.m_dataMem.size()}, // Start address
                {MatCoreInstDefn::M1, matrixState.m_regIdx[bx][by]}
            }});

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
    const auto& matrixState = m_dataMatrixState.at(handle);
    auto& coreState = m_procState.matCores[matrixState.m_coreIdx];

    // Save to dataMem
    std::vector<std::vector<size_t>> dataAddr(matrixState.m_shape.x,
        std::vector<size_t>(matrixState.m_shape.y, 0));

    for (size_t bx = 0;bx < matrixState.m_shape.x;bx++) {
        for (size_t by = 0;by < matrixState.m_shape.y;by++) {
            // Record address
            dataAddr[bx][by] = coreState.m_dataMem.size();

            // Add save instruction
            coreState.m_prog.append({MatCoreInstDefn::STORE_MAT, {
                {MatCoreInstDefn::ADDR, dataAddr[bx][by]},
                {MatCoreInstDefn::M1, matrixState.m_regIdx[bx][by]}
            }});

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
