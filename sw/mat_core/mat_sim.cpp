#include "mat_program.h"
#include "mat_sim.h"
#include "error.h"
#include <sstream>


// Init simulation engine
MatCoreSimEngine::MatCoreSimEngine(const MatCoreProgram& prog, const MatCoreParam& param)
: m_prog(prog), m_param(param), m_state(State::INIT), m_pc(0) {}

// Test if completed
bool MatCoreSimEngine::isDone() const {
    return m_state == State::STOP;
}

// Simulate one step
void MatCoreSimEngine::simulateStep() {
    auto const& inst = m_prog[m_pc];

    State next_state = m_state;
    bool next_inst_proceed = false;
    bool next_opcode_is_unit = m_pc + 1 < m_prog.size() &&
        (m_prog[m_pc + 1].opcode == MatCoreInstDefn::SET_WEIGHT ||
         m_prog[m_pc + 1].opcode == MatCoreInstDefn::MULTIPLY);

    switch (m_state) {
        case State::INIT: {
            next_state = State::READY;
            break;
        }
        case State::READY: {
            switch (inst.opcode) {
                case MatCoreInstDefn::SET_WEIGHT:
                case MatCoreInstDefn::MULTIPLY: {
                    next_state = State::P0XX;
                    m_diag_progress_counter = 0;
                    break;
                }
                case MatCoreInstDefn::LOAD_MAT:
                case MatCoreInstDefn::STORE_MAT: {
                    next_state = State::ACCESS_MEM;
                    m_diag_progress_counter = 0;
                    break;
                }
                case MatCoreInstDefn::HALT: {
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
        case State::NEXT: {
            next_inst_proceed = true;
            next_state = State::READY;
            break;
        }
        case State::STOP: {
            next_state = State::STOP;
            break;
        }
        case State::ACCESS_MEM: {
            if (m_diag_progress_counter == m_param.width - 1) {
                next_state = State::NEXT;
            } else {
                next_state = State::ACCESS_MEM;
                m_diag_progress_counter++;
            }
            break;
        }
        case State::P0XX: {
            if (m_diag_progress_counter == m_param.width -1) {
                if (next_opcode_is_unit) {
                    next_state = State::P01X;
                    next_inst_proceed = true;
                } else {
                    next_state = State::PX0X;
                }
                m_diag_progress_counter = 0;
            } else {
                next_state = State::P0XX;
                m_diag_progress_counter++;
            }
            break;
        }
        case State::P01X: {
            if (m_diag_progress_counter == m_param.width -1) {
                if (next_opcode_is_unit) {
                    next_state = State::P012;
                    next_inst_proceed = true;
                } else {
                    next_state = State::PX01;
                }
                m_diag_progress_counter = 0;
            } else {
                next_state = State::P01X;
                m_diag_progress_counter++;
            }
            break;
        }
        case State::PX0X: {
            if (m_diag_progress_counter == m_param.width -1) {
                next_state = State::PXX0;
                m_diag_progress_counter = 0;
            } else {
                next_state = State::PX0X;
                m_diag_progress_counter++;
            }
            break;
        }
        case State::PXX0: {
            if (m_diag_progress_counter == m_param.width -1) {
                next_state = State::NEXT;
            } else {
                next_state = State::PXX0;
                m_diag_progress_counter++;
            }
            break;
        }
        case State::PX01: {
            if (m_diag_progress_counter == m_param.width -1) {
                next_state = State::PXX0;
                m_diag_progress_counter = 0;
            } else {
                next_state = State::PX01;
                m_diag_progress_counter++;
            }
            break;
        }
        case State::P012: {
            if (m_diag_progress_counter == m_param.width -1) {
                if (next_opcode_is_unit) {
                    next_state = State::P012;
                    next_inst_proceed = true;
                } else {
                    next_state = State::PX01;
                }
                m_diag_progress_counter = 0;
            } else {
                next_state = State::P012;
                m_diag_progress_counter++;
            }
            break;
        }
    }

    // Update internal state
    m_state = next_state;
    if (next_inst_proceed) {
        m_pc++;
        if (m_pc >= m_prog.size()) {
            FatalError("MatCore instruction memory out of bound");
        }
    }
}
