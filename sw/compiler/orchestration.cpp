#include "orchestration.h"
#include "error.h"

#include <string>
#include <fstream>
#include <iomanip>

// Construct orchestrator
Orchestrator::Orchestrator(const OrchestratorParam& param)
: m_param(param),
  m_matCoreStatus(param.matCoreCount),
  m_vecCoreStatus(param.vecCoreCount),
  m_matrixHandleCount(0)
{}


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
Orchestrator::MatrixState::MatrixState(size_t bx, size_t by)
: bx(bx), by(by), activeRegIdx(bx, std::vector<size_t>(by, 0))
{}


// Data operations
// TODO: The simpler version allocates everything to matcore 0
// In this case veccore are unused by orchestrator

// Allocate a matrix
Orchestrator::MatrixHandle Orchestrator::dataMatrixAllocate(const Matrix& matrix) {
    MatrixHandle handle = m_matrixHandleCount++;

    // Determine register size
    size_t bx = (matrix.x + m_param.width - 1) / m_param.width,
           by = (matrix.y + m_param.width - 1) / m_param.width;
    auto res = m_dataMatrixStatus.emplace(handle, MatrixState(bx, by));
    if (!res.second) {
        FatalError("Duplicate MatrixHandle");
    }

    // Initialize matrixState
    auto& matrixState = res.first->second;
    matrixState.coreIdx = 0;

    for (size_t i = 0;i < bx;i++) {
        for (size_t j = 0;j < by;j++) {
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

    // Return handle
    return handle;
}

// Deallocate a mtrix
void Orchestrator::dataMatrixDeallocate(MatrixHandle handle) {
    // Release registers
    if (m_dataMatrixStatus.count(handle) == 0) {
        FatalError("Orchestrator dealloc " + std::to_string(handle) + " not exist");
    }
    auto& matrixState = m_dataMatrixStatus.at(handle);
    size_t bx = matrixState.bx,
           by = matrixState.by;
    for (size_t i = 0;i < bx;i++) {
        for (size_t j = 0;j < by;j++) {
            size_t regIdx = matrixState.activeRegIdx[i][j];
            m_matCoreStatus[matrixState.coreIdx].freeRegIdx.insert(regIdx);
        }
    }
    m_dataMatrixStatus.erase(handle);
}

// Bind to constants
void Orchestrator::dataMatrixBindConstant(MatrixHandle h, const float* data) {
}
