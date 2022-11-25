#ifndef __VEC_SIM_H__
#define __VEC_SIM_H__

#include "program.h"
#include "vec_program.h"


// VecCore parameters
struct VecCoreParam {
    size_t width = 16;
};


// VecCore simulation engine
class VecCoreSimEngine {
public:
    VecCoreSimEngine(const VecCoreProgram&, const VecCoreParam&);

    void simulateStep();
    bool isDone() const;

private:
    enum class State {
        INIT, READY, NEXT, STOP,
        READREG,
    };
    VecCoreProgram m_prog;
    VecCoreParam m_param;

    // Internal state
    State m_state;
    size_t m_pc;
};


#endif
