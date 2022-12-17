#include "mat_program.h"
#include "mat_sim.h"
#include "error.h"
#include <sstream>
#include <iostream>


// Init simulation engine
MatCoreSimEngine::MatCoreSimEngine(const MatCoreProgram& prog, const MatCoreParam& param, SwitchSimEngine* p_switch)
: m_prog(prog), m_param(param),
  m_state(State::INIT), m_pc(0),
  m_memoryPenaltyCounter(0),
  p_switch(p_switch)
{
    // Add memory penalty
    if (m_pc < m_prog.size()) {
        // Test if the first instruction is load/store
        if (MatCoreInstDefn::isMemoryOperation(m_prog[m_pc].opcode)) {
            m_memoryPenaltyCounter = m_param.memoryPenalty;
        }
    }
}

// Test if completed
bool MatCoreSimEngine::isDone() const {
    return m_state == State::STOP;
}

// Simulate one step
void MatCoreSimEngine::simulateStep() {
    // Check if terminate without halt is allowed
    if (m_pc >= m_prog.size()) {
        if (m_param.allowNoHalt) {
            m_state = State::STOP;
        } else {
            FatalError("MatCore instruction memory out of bound");
        }
    }

    // Fetch instruction
    auto const& inst = m_prog[m_pc];

    // Update stat
    m_instCycleStat[inst.opcode]++;

    // Check if we need to wait for memory penalty
    if (m_memoryPenaltyCounter != 0) {
        m_memoryPenaltyCounter--;
        return;
    }

    // State machine
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
        case State::WAIT_SWITCH: // WAIT_SWITCH only branch to send/recv ops
        case State::READY: {
            switch (inst.opcode) {
                case MatCoreInstDefn::SET_WEIGHT:
                case MatCoreInstDefn::MULTIPLY: {
                    next_state = State::P0XX;
                    m_diag_progress_counter = 0;
                    break;
                }
                case MatCoreInstDefn::COPY:
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
                case MatCoreInstDefn::SEND_ROW:
                case MatCoreInstDefn::SEND_COL:
                case MatCoreInstDefn::SEND_SCALAR:
                case MatCoreInstDefn::SEND_DIAG: {
                    // Send op
                    if (p_switch == nullptr) {
                        FatalError("Null pointer to switch");
                    }
                    bool send_ok = p_switch->sendRequest(m_param.core_self, inst.operands.at(MatCoreInstDefn::CORE_IDX));
                    next_state = send_ok ? State::NEXT : State::WAIT_SWITCH;
                    break;
                }
                case MatCoreInstDefn::RECV_ROW:
                case MatCoreInstDefn::RECV_COL:
                case MatCoreInstDefn::RECV_SCALAR:
                case MatCoreInstDefn::RECV_DIAG:
                case MatCoreInstDefn::RECV_DIAG1:
                case MatCoreInstDefn::RECV_DIAG2: {
                    // Recv op
                    if (p_switch == nullptr) {
                        FatalError("Null pointer to switch");
                    }
                    bool recv_ok = p_switch->recvRequest(m_param.core_self, inst.operands.at(MatCoreInstDefn::CORE_IDX));
                    next_state = recv_ok ? State::NEXT : State::WAIT_SWITCH;
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

        // Add memory penalty
        if (m_pc < m_prog.size()) {
            // Test if the next instruction is load/store
            if (MatCoreInstDefn::isMemoryOperation(m_prog[m_pc].opcode)) {
                m_memoryPenaltyCounter = m_param.memoryPenalty;
            }
        }
    }
}

void MatCoreSimEngine::printStat() const {
    for (const auto& [opcode, count] : m_instCycleStat) {
        std::cout << MatCoreInstDefn::getOpcodeName(opcode) << ": " << count << std::endl;
    }
}
