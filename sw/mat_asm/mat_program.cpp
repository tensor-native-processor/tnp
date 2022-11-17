#include "mat_program.h"
#include "error.h"
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

// Get type for each operand
const std::map<MatInstruction::Operand, MatInstruction::Type> MatInstruction::operandType = {
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
size_t& MatInstructionSize::operator[](MatInstruction::Type type) {
    return size[type];
}
const size_t& MatInstructionSize::operator[] (MatInstruction::Type type) const {
    return size.at(type);
}

// Find opcode
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
    FatalError("No opcode with name " + opName);
}
MatInstruction::Type MatInstruction::getOperandType(Operand opr) {
    return operandType.at(opr);
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

// Convert MatProgram to/from binary
TNPProgramBinary MatProgram::toBinary() const {
    TNPProgramBinary bin;
    for (auto const &inst : m_instructions) {
        auto opcBin = encodeBinary(inst.opcode, m_isizes[MatInstruction::OPCODE_TYPE]);
        bin.insert(bin.end(), opcBin.begin(), opcBin.end());

        auto operands = MatInstruction::getOpcodeOperands(inst.opcode);
        for (auto const& opr : operands) {
            auto oprBin = encodeBinary(inst.operands.at(opr), m_isizes[MatInstruction::getOperandType(opr)]);
            bin.insert(bin.end(), oprBin.begin(), oprBin.end());
        }
    }
    return bin;
}

void MatProgram::fromBinary(const TNPProgramBinary& bin) {
    // Clear existing program
    m_instructions.clear();
    MatInstruction inst;

    auto iter = bin.begin();
    while (iter != bin.end()) {
        size_t size;

        // Fetch opcode
        size = m_isizes[MatInstruction::OPCODE_TYPE];
        inst.opcode = (MatInstruction::Opcode)decodeBinary(TNPProgramBinary(iter, iter + size));
        iter += size;

        // Fetch operands
        auto operands = MatInstruction::getOpcodeOperands(inst.opcode);
        for (auto const& opr : operands) {
            size = m_isizes[MatInstruction::getOperandType(opr)];
            inst.operands[opr] = decodeBinary(TNPProgramBinary(iter, iter + size));
            iter += size;
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
