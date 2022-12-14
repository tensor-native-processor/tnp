#ifndef __VEC_SIM_H__
#define __VEC_SIM_H__

#include "program.h"
#include "vec_program.h"
#include "switch.h"


// VecCore parameters
struct VecCoreParam {
    size_t width = 16;
    size_t core_self = 0;
    bool allowNoHalt = false;
    size_t memoryPenalty = 0;
};


// VecCore simulation engine
class VecCoreSimEngine {
public:
    VecCoreSimEngine(const VecCoreProgram&, const VecCoreParam&, SwitchSimEngine* p_switch = nullptr);

    void simulateStep();
    bool isDone() const;

    void printStat() const;

private:
    enum class State {
        INIT, READY, NEXT, STOP,
        READREG,
        WAIT_SWITCH,
    };
    VecCoreProgram m_prog;
    VecCoreParam m_param;

    // Internal state
    State m_state;
    size_t m_pc;

    // Memory penalty
    size_t m_memoryPenaltyCounter;

    // Pointer to switch
    SwitchSimEngine* p_switch;

    // Statistics
    std::map<VecCoreInstDefn::Opcode, size_t> m_instCycleStat;
};


#endif
