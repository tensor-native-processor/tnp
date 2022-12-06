#include "orchestration.h"

#include <string>

// Construct orchestrator
Orchestrator::Orchestrator(const OrchestratorParam& param)
: m_param(param),
  m_matProgs(param.matCoreCount),
  m_vecProgs(param.vecCoreCount)
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
    for (size_t id = 0;id < m_matProgs.size();id++) {
        MatCoreInst matInst;
        matInst.opcode = MatCoreInstDefn::HALT;
        m_matProgs[id].append(matInst);
    }
    for (size_t id = 0;id < m_vecProgs.size();id++) {
        VecCoreInst vecInst;
        vecInst.opcode = VecCoreInstDefn::HALT;
        m_vecProgs[id].append(vecInst);
    }

    // Save to files
    for (size_t id = 0;id < m_matProgs.size();id++) {
        std::string filename = "inst_mem" + std::to_string(getMatCoreID(id)) + ".txt";
        SaveProgram(m_matProgs[id].toBinary(), filename);
    }
    for (size_t id = 0;id < m_vecProgs.size();id++) {
        std::string filename = "inst_mem" + std::to_string(getVecCoreID(id)) + ".txt";
        SaveProgram(m_vecProgs[id].toBinary(), filename);
    }
}
