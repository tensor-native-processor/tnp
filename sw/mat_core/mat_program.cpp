#include "mat_program.h"
#include "error.h"
#include <sstream>

// Instruction operand map
const std::map<MatCoreInstDefn::Opcode, std::vector<MatCoreInstDefn::Operand>> MatCoreInstDefn::operandMap = {
    {SET_WEIGHT,        {M1}},
    {MULTIPLY,          {Md, M1}},
    {TRANSPOSE,         {M1}},
    {XFLIP,             {M1}},
    {YFLIP,             {M1}},

    {LOAD_MAT,          {ADDR, M1}},
    {LOAD_ROW,          {ADDR, M1, ROW_IDX}},
    {LOAD_COL,          {ADDR, M1, COL_IDX}},
    {LOAD_SCALAR,       {ADDR, M1, ROW_IDX, COL_IDX}},
    {STORE_MAT,         {ADDR, M1}},
    {STORE_ROW,         {ADDR, M1, ROW_IDX}},
    {STORE_COL,         {ADDR, M1, COL_IDX}},
    {STORE_SCALAR,      {ADDR, M1, ROW_IDX, COL_IDX}},

    {SEND_ROW,          {CORE_IDX, M1, ROW_IDX}},
    {SEND_COL,          {CORE_IDX, M1, COL_IDX}},
    {SEND_SCALAR,       {CORE_IDX, M1, ROW_IDX, COL_IDX}},
    {SEND_DIAG,         {CORE_IDX, M1, M2, DIAG_IDX}},

    {RECV_ROW,          {CORE_IDX, M1, ROW_IDX}},
    {RECV_COL,          {CORE_IDX, M1, COL_IDX}},
    {RECV_SCALAR,       {CORE_IDX, M1, ROW_IDX, COL_IDX}},
    {RECV_DIAG,         {CORE_IDX, M1, M2, DIAG_IDX}},
    {RECV_DIAG1,        {CORE_IDX, M1, DIAG_IDX}},
    {RECV_DIAG2,        {CORE_IDX, M1, DIAG_IDX}},
    {HALT,              {}},
};

// Instruction operator name
const std::map<MatCoreInstDefn::Opcode, std::string> MatCoreInstDefn::opcodeName = {
    {SET_WEIGHT,        "SET_WEIGHT"},
    {MULTIPLY,          "MULTIPLY"},
    {TRANSPOSE,         "TRANSPOSE"},
    {XFLIP,             "XFLIP"},
    {YFLIP,             "YFLIP"},

    {LOAD_MAT,          "LOAD_MAT"},
    {LOAD_ROW,          "LOAD_ROW"},
    {LOAD_COL,          "LOAD_COL"},
    {LOAD_SCALAR,       "LOAD_SCALAR"},

    {STORE_MAT,         "STORE_MAT"},
    {STORE_ROW,         "STORE_ROW"},
    {STORE_COL,         "STORE_COL"},
    {STORE_SCALAR,      "STORE_SCALAR"},

    {SEND_ROW,          "SEND_ROW"},
    {SEND_COL,          "SEND_COL"},
    {SEND_SCALAR,       "SEND_SCALAR"},
    {SEND_DIAG,         "SEND_DIAG"},

    {RECV_ROW,          "RECV_ROW"},
    {RECV_COL,          "RECV_COL"},
    {RECV_SCALAR,       "RECV_SCALAR"},
    {RECV_DIAG,         "RECV_DIAG"},
    {RECV_DIAG1,        "RECV_DIAG1"},
    {RECV_DIAG2,        "RECV_DIAG2"},
    {HALT,              "HALT"},
};

// Get type for each operand
const std::map<MatCoreInstDefn::Operand, MatCoreInstDefn::Type> MatCoreInstDefn::operandType = {
    {ADDR,              MEM_ADDR_TYPE},
    {CORE_IDX,          CORE_IDX_TYPE},
    {Md,                REG_ADDR_TYPE},
    {M1,                REG_ADDR_TYPE},
    {M2,                REG_ADDR_TYPE},
    {ROW_IDX,           WIDTH_IDX_TYPE},
    {COL_IDX,           WIDTH_IDX_TYPE},
    {DIAG_IDX,          WIDTH_IDX_TYPE},
};

// MatInstructionSize []
size_t& MatCoreInstSize::operator[](MatCoreInstDefn::Type type) {
    return size[type];
}
const size_t& MatCoreInstSize::operator[] (MatCoreInstDefn::Type type) const {
    return size.at(type);
}

// Find opcode
std::vector<MatCoreInstDefn::Operand> MatCoreInstDefn::getOpcodeOperands(Opcode op) {
    return operandMap.at(op);
}

std::string MatCoreInstDefn::getOpcodeName(Opcode op) {
    return opcodeName.at(op);
}
MatCoreInstDefn::Opcode MatCoreInstDefn::findOpcodeByName(std::string opName) {
    for (auto const& [key, val] : opcodeName) {
        if (val == opName) {
            return key;
        }
    }
    FatalError("No opcode with name " + opName);
}
MatCoreInstDefn::Type MatCoreInstDefn::getOperandType(Operand opr) {
    return operandType.at(opr);
}

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
