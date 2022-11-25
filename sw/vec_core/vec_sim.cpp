#include "vec_program.h"
#include "vec_sim.h"
#include "error.h"
#include <sstream>

// Init simulation engine
VecCoreSimEngine::VecCoreSimEngine(const VecCoreProgram& prog, const VecCoreParam& param)
: m_prog(prog), m_param(param), m_state(State::INIT), m_pc(0) {}

// Test if completed
bool VecCoreSimEngine::isDone() const {
    return m_state == State::STOP;
}

// Simulate one step
void VecCoreSimEngine::simulateStep() {
    auto const& inst = m_prog[m_pc];

    State next_state = m_state;
    bool next_inst_proceed = false;

    switch (m_state) {
        case State::INIT: {
            next_state = State::READY;
            break;
        }
        case State::READY: {
            switch (inst.opcode) {
                case VecCoreInstDefn::ADD:
                case VecCoreInstDefn::SUB:
                case VecCoreInstDefn::DOT:
                case VecCoreInstDefn::DELTA:
                case VecCoreInstDefn::SCALE: {
                    next_state = State::READREG;
                    break;
                }
                case VecCoreInstDefn::HALT: {
                    next_state = State::STOP;
                    break;
                }
                default: {
                    // All other opcodes lead to NEXT
                    next_state = State::NEXT;
                    break;
                }
            }
            break;
        }
        case State::READREG: {
            next_state = State::NEXT;
            break;
        }
        case State::NEXT: {
            next_inst_proceed = true;
            next_state = State::READY;
            break;
        }
        case State::STOP: {
            next_state = State::STOP;
            break;
        }
    }

    // Update internal state
    m_state = next_state;
    if (next_inst_proceed) {
        m_pc++;
        if (m_pc >= m_prog.size()) {
            FatalError("VecCore instruction memory out of bound");
        }
    }
}
