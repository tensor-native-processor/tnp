#ifndef __MAT_SIM_H__
#define __MAT_SIM_H__

#include "program.h"
#include "mat_program.h"
#include "switch.h"

// MatCore parameters
struct MatCoreParam {
    size_t width = 16;
    size_t core_self = 0;
    bool allowNoHalt = false;
    size_t memoryPenalty = 0;
};


// MatCore simulation engine
class MatCoreSimEngine {
public:
    MatCoreSimEngine(const MatCoreProgram&, const MatCoreParam&, SwitchSimEngine* p_switch = nullptr);

    void simulateStep();
    bool isDone() const;

    void printStat() const;

private:
    enum class State {
        INIT, READY, NEXT, STOP,
        P0XX, P01X, P012,
        PX0X, PX01,
        PXX0,
        ACCESS_MEM,
        WAIT_SWITCH
    };
    MatCoreProgram m_prog;
    MatCoreParam m_param;

    // Internal state
    State m_state;
    size_t m_pc;
    size_t m_diag_progress_counter;

    // Memory penalty
    size_t m_memoryPenaltyCounter;

    // Pointer to switch
    SwitchSimEngine* p_switch;

    // Statistics
    std::map<MatCoreInstDefn::Opcode, size_t> m_instCycleStat;
};


#endif
