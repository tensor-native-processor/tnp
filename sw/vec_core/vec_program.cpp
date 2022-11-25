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
