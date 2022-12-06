#include "orchestration.h"

#include <string>
#include <fstream>
#include <iomanip>

// Construct orchestrator
Orchestrator::Orchestrator(const OrchestratorParam& param)
: m_param(param),
  m_matCoreStatus(param.matCoreCount),
  m_vecCoreStatus(param.vecCoreCount)
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


// Data operations
// TODO: The simpler version allocates everything to matcore 0
// In this case veccore are unused by orchestrator

// Allocate a matrix
Orchestrator::MatrixHandle Orchestrator::dataMatrixAllocate(const Matrix& m) {
}

// Deallocate a mtrix
void Orchestrator::dataMatrixDeallocate(MatrixHandle h) {
}

// Bind to constants
void Orchestrator::dataMatrixBindConstant(MatrixHandle h, const float* data) {
}
