#include "vec_program.h"
#include "error.h"
#include <sstream>

// Instruction operand map
const std::map<VecCoreInstDefn::Opcode, std::vector<VecCoreInstDefn::Operand>> VecCoreInstDefn::operandMap = {
    {ADD,               {Vd, V1, V2}},
    {SUB,               {Vd, V1, V2}},
    {DOT,               {Vd, V1, V2}},
    {SCALE,             {Vd, V1, V2, VEC_IDX}},
    {DELTA,             {Vd, V1, V2, VEC_IDX}},
    {ACT_SIGMOID,       {Vd, V1}},
    {ACT_TANH,          {Vd, V1}},
    {ACT_RELU,          {Vd, V1}},
    {CLEAR,             {V1}},
    {COPY,              {Vd, V1}},

    {LOAD_VEC,          {ADDR, V1}},
    {LOAD_SCALAR,       {ADDR, V1, VEC_IDX}},
    {STORE_VEC,         {ADDR, V1}},
    {STORE_SCALAR,      {ADDR, V1, VEC_IDX}},

    {SEND_VEC,          {CORE_IDX, V1}},
    {SEND_SCALAR,       {CORE_IDX, V1, VEC_IDX}},
    {RECV_VEC,          {CORE_IDX, V1}},
    {RECV_SCALAR,       {CORE_IDX, V1, VEC_IDX}},

    {HALT,              {}},
};

// Instruction operator name
const std::map<VecCoreInstDefn::Opcode, std::string> VecCoreInstDefn::opcodeName = {
    {ADD,               "ADD"},
    {SUB,               "SUB"},
    {DOT,               "DOT"},
    {SCALE,             "SCALE"},
    {DELTA,             "DELTA"},
    {ACT_SIGMOID,       "ACT_SIGMOID"},
    {ACT_TANH,          "ACT_TANH"},
    {ACT_RELU,          "ACT_RELU"},

    {CLEAR,             "CLEAR"},
    {COPY,              "COPY"},

    {LOAD_VEC,          "LOAD_VEC"},
    {LOAD_SCALAR,       "LOAD_SCALAR"},
    {STORE_VEC,         "STORE_VEC"},
    {STORE_SCALAR,      "STORE_SCALAR"},

    {SEND_VEC,          "SEND_VEC"},
    {SEND_SCALAR,       "SEND_SCALAR"},
    {RECV_VEC,          "RECV_VEC"},
    {RECV_SCALAR,       "RECV_SCALAR"},

    {HALT,              "HALT"},
};

// Get type for each operand
const std::map<VecCoreInstDefn::Operand, VecCoreInstDefn::Type> VecCoreInstDefn::operandType = {
    {ADDR,              MEM_ADDR_TYPE},
    {CORE_IDX,          CORE_IDX_TYPE},
    {Vd,                REG_ADDR_TYPE},
    {V1,                REG_ADDR_TYPE},
    {V2,                REG_ADDR_TYPE},
    {VEC_IDX,           WIDTH_IDX_TYPE},
};

// VecInstructionSize []
size_t& VecCoreInstSize::operator[](VecCoreInstDefn::Type type) {
    return size[type];
}
const size_t& VecCoreInstSize::operator[] (VecCoreInstDefn::Type type) const {
    return size.at(type);
}

// Find opcode
std::vector<VecCoreInstDefn::Operand> VecCoreInstDefn::getOpcodeOperands(Opcode op) {
    return operandMap.at(op);
}

std::string VecCoreInstDefn::getOpcodeName(Opcode op) {
    return opcodeName.at(op);
}
VecCoreInstDefn::Opcode VecCoreInstDefn::findOpcodeByName(std::string opName) {
    for (auto const& [key, val] : opcodeName) {
        if (val == opName) {
            return key;
        }
    }
    FatalError("No opcode with name " + opName);
}
VecCoreInstDefn::Type VecCoreInstDefn::getOperandType(Operand opr) {
    return operandType.at(opr);
}

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
