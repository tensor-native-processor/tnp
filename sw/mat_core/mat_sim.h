#ifndef __MAT_SIM_H__
#define __MAT_SIM_H__

#include "program.h"
#include "mat_program.h"

// MatCore parameters
struct MatCoreParam {
    size_t width = 16;
};


// MatCore simulation engine
class MatCoreSimEngine {
public:
    MatCoreSimEngine(const MatCoreProgram&, const MatCoreParam&);

    void simulateStep();
    bool isDone() const;

private:
    enum class State {
        INIT, READY, NEXT, STOP,
        P0XX, P01X, P012,
        PX0X, PX01,
        PXX0,
        ACCESS_MEM
    };
    MatCoreProgram m_prog;
    MatCoreParam m_param;

    // Internal state
    State m_state;
    size_t m_pc;
    size_t m_diag_progress_counter;
};


#endif
