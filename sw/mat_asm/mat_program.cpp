#include "mat_program.h"
#include "error.h"
#include <sstream>

// Instruction operand map
const std::map<MatCoreInstDefn::Opcode, std::vector<MatCoreInstDefn::Operand>> MatCoreInstDefn::operandMap = {
    {SET_WEIGHT,        {M1}},
    {MULTIPLY,          {Md, M1}},
    {TRANSPOSE,         {M1}},
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
    {SEND_DIAG,         {CORE_IDX, M1, M2, DIAG_IDX}},
    {RECV_ROW,          {CORE_IDX, M1, ROW_IDX}},
    {RECV_COL,          {CORE_IDX, M1, COL_IDX}},
    {RECV_SCALAR,       {CORE_IDX, M1, ROW_IDX, COL_IDX, ELEM_IDX}},
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
    {ELEM_IDX,          WIDTH_IDX_TYPE},
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
