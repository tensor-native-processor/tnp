#include "mat_program.h"
#include <sstream>

// Instruction operand map
const std::map<MatInstruction::Opcode, std::vector<MatInstruction::Operand>> MatInstruction::operandMap = {
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
const std::map<MatInstruction::Opcode, std::string> MatInstruction::opcodeName = {
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

/*
// Operands in different categories
const std::vector<MatInstruction::Operand> MatInstruction::matMemAddrOperands = {
    ADDR
};
const std::vector<MatInstruction::Operand> MatInstruction::matCoreIdxOperands = {
    CORE_IDX
};
const std::vector<MatInstruction::Operand> MatInstruction::matRegAddrOperands = {
    Md, M1, M2
};
const std::vector<MatInstruction::Operand> MatInstruction::matWidthSizeOperands = {
    ROW_IDX, COL_IDX, DIAG_IDX, ELEM_IDX
};
*/

std::vector<MatInstruction::Operand> MatInstruction::getOpcodeOperands(Opcode op) {
    return operandMap.at(op);
}

std::string MatInstruction::getOpcodeName(Opcode op) {
    return opcodeName.at(op);
}
MatInstruction::Opcode MatInstruction::findOpcodeByName(std::string opName) {
    for (auto const& [key, val] : opcodeName) {
        if (val == opName) {
            return key;
        }
    }
//    FatalError("No opcode with name " + opName);
}

// Convert MatProgram to/from text
std::string MatProgram::toText() const {
    std::ostringstream oss;
    for (auto const &inst : m_instructions) {
        oss << MatInstruction::getOpcodeName(inst.opcode) << " ";
        auto operands = MatInstruction::getOpcodeOperands(inst.opcode);

        for (auto const& opr : operands) {
            oss << inst.operands.at(opr) << " ";
        }
        oss << "\n";
    }
    return oss.str();
}

void MatProgram::fromText(const std::string& str) {
    std::istringstream iss(str);
    // Clear existing program
    m_instructions.clear();
    MatInstruction inst;
    std::string opName;
    while (iss >> opName) {
        inst.opcode = MatInstruction::findOpcodeByName(opName);
        auto operands = MatInstruction::getOpcodeOperands(inst.opcode);

        for (auto const& opr : operands) {
            iss >> inst.operands.at(opr);
        }
        m_instructions.push_back(inst);
    }
}

// Append one instruction to mat program
void MatProgram::append(const MatInstruction& inst) {
    m_instructions.push_back(inst);
}

// Encode and decode binary content
TNPProgramBinary MatProgram::encodeBinary(MatValue_t value, size_t size) {
	TNPProgramBinary binary;
	for (size_t i = 0;i < size;i++) {
        std::byte b = std::byte(value & 0xFF);
		binary.push_back(b);
		value >>= 8;
	}
	return binary;
}

MatValue_t MatProgram::decodeBinary(const TNPProgramBinary& binary) {
	MatValue_t value = 0;
	for (size_t i = 0;i < binary.size();i++) {
		MatValue_t byte = (MatValue_t)(binary[i]);
		value |= byte << 8 * i;
	}
	return value;
}
