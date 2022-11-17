#include "mat_program.h"
#include <sstream>

// Instruction operand map
const std::map<MatInstruction::Operator, std::set<MatInstruction::OperandTag>> MatInstruction::operandMap = {
    {SET_WEIGHT,        {T_M1}},
    {MULTIPLY,          {T_Md, T_M1}},
    {TRANSPOSE,         {T_M1}},
    {LOAD_MAT,          {T_addr, T_M1}},
    {LOAD_ROW,          {T_addr, T_M1, T_row_idx}},
    {LOAD_COL,          {T_addr, T_M1, T_col_idx}},
    {LOAD_SCALAR,       {T_addr, T_M1, T_row_idx, T_col_idx}},
    {STORE_MAT,         {T_addr, T_M1}},
    {STORE_ROW,         {T_addr, T_M1, T_row_idx}},
    {STORE_COL,         {T_addr, T_M1, T_col_idx}},
    {STORE_SCALAR,      {T_addr, T_M1, T_row_idx, T_col_idx}},
    {SEND_ROW,          {T_core_idx, T_M1, T_row_idx}},
    {SEND_COL,          {T_core_idx, T_M1, T_col_idx}},
    {SEND_DIAG,         {T_core_idx, T_M1, T_M2, T_diag_idx}},
    {RECV_ROW,          {T_core_idx, T_M1, T_row_idx}},
    {RECV_COL,          {T_core_idx, T_M1, T_col_idx}},
    {RECV_SCALAR,       {T_core_idx, T_M1, T_row_idx, T_col_idx, T_elem_idx}},
    {RECV_DIAG,         {T_core_idx, T_M1, T_M2, T_diag_idx}},
    {RECV_DIAG1,        {T_core_idx, T_M1, T_diag_idx}},
    {RECV_DIAG2,        {T_core_idx, T_M1, T_diag_idx}},
    {HALT,              {}},

    {ILLEGAL_OP,        {}},
};

// Instruction operator name
const std::map<MatInstruction::Operator, std::string> MatInstruction::operatorName = {
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

    {ILLEGAL_OP,        "ILLEGAL_OP"},
};


std::set<MatInstruction::OperandTag> MatInstruction::operands(Operator op) {
    return operandMap.at(op);
}

std::string MatInstruction::getOperatorName(Operator op) {
    return operatorName.at(op);
}
MatInstruction::Operator MatInstruction::findOperatorByName(std::string opName) {
    for (auto const& [key, val] : operatorName) {
        if (val == opName) {
            return key;
        }
    }
    return ILLEGAL_OP;
}

// Convert MatProgram to/from text
std::string MatProgram::toText() const {
    std::ostringstream oss;
    for (auto const &inst : m_instructions) {
        oss << MatInstruction::getOperatorName(inst.op) << " ";
        auto operands = MatInstruction::operands(inst.op);

        // Mem address
        if (operands.count(MatInstruction::T_addr)) oss << inst.addr << " ";
        // Core index
        if (operands.count(MatInstruction::T_core_idx)) oss << inst.core_idx << " ";
        // Mat reg
        if (operands.count(MatInstruction::T_Md)) oss << inst.Md << " ";
        if (operands.count(MatInstruction::T_M1)) oss << inst.M1 << " ";
        if (operands.count(MatInstruction::T_M2)) oss << inst.M2 << " ";
        // Width index
        if (operands.count(MatInstruction::T_row_idx)) oss << inst.row_idx << " ";
        if (operands.count(MatInstruction::T_col_idx)) oss << inst.col_idx << " ";
        if (operands.count(MatInstruction::T_diag_idx)) oss << inst.diag_idx << " ";
        if (operands.count(MatInstruction::T_elem_idx)) oss << inst.elem_idx << " ";

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
        inst.op = MatInstruction::findOperatorByName(opName);
        auto operands = MatInstruction::operands(inst.op);

        // Mem address
        if (operands.count(MatInstruction::T_addr)) iss >> inst.addr;
        // Core index
        if (operands.count(MatInstruction::T_core_idx)) iss >> inst.core_idx;
        // Mat reg
        if (operands.count(MatInstruction::T_Md)) iss >> inst.Md;
        if (operands.count(MatInstruction::T_M1)) iss >> inst.M1;
        if (operands.count(MatInstruction::T_M2)) iss >> inst.M2;
        // Width index
        if (operands.count(MatInstruction::T_row_idx)) iss >> inst.row_idx;
        if (operands.count(MatInstruction::T_col_idx)) iss >> inst.col_idx;
        if (operands.count(MatInstruction::T_diag_idx)) iss >> inst.diag_idx;
        if (operands.count(MatInstruction::T_elem_idx)) iss >> inst.elem_idx;

        m_instructions.push_back(inst);
    }
}

// Convert MatProgram to/from binary
TNPProgramBinary MatProgram::toBinary() const {
    TNPProgramBinary output;
    for (auto const &inst : m_instructions) {
        auto opcodeBin = encodeBinary(inst.op, m_formatConfig.MatInstSize);
        output.insert(output.end(), opcodeBin.begin(), opcodeBin.end());

        auto operands = MatInstruction::operands(inst.op);

        // Mem address
        if (operands.count(MatInstruction::T_addr)) {
            auto opBin = encodeBinary(inst.addr, m_formatConfig.MatMemAddrSize);
            output.insert(output.end(), opBin.begin(), opBin.end());
        }
        // Core index
        if (operands.count(MatInstruction::T_core_idx)) {
            auto opBin = encodeBinary(inst.core_idx, m_formatConfig.MatCoreIdxSize);
            output.insert(output.end(), opBin.begin(), opBin.end());
        }
        // Mat reg
        if (operands.count(MatInstruction::T_Md)) {
            auto opBin = encodeBinary(inst.Md, m_formatConfig.MatRegAddrSize);
            output.insert(output.end(), opBin.begin(), opBin.end());
        }
        if (operands.count(MatInstruction::T_M1)) {
            auto opBin = encodeBinary(inst.M1, m_formatConfig.MatRegAddrSize);
            output.insert(output.end(), opBin.begin(), opBin.end());
        }
        if (operands.count(MatInstruction::T_M2)) {
            auto opBin = encodeBinary(inst.M2, m_formatConfig.MatRegAddrSize);
            output.insert(output.end(), opBin.begin(), opBin.end());
        }
        // Width index
        if (operands.count(MatInstruction::T_row_idx)) {
            auto opBin = encodeBinary(inst.row_idx, m_formatConfig.MatWidthIdxSize);
            output.insert(output.end(), opBin.begin(), opBin.end());
        }
        if (operands.count(MatInstruction::T_col_idx)) {
            auto opBin = encodeBinary(inst.col_idx, m_formatConfig.MatWidthIdxSize);
            output.insert(output.end(), opBin.begin(), opBin.end());
        }
        if (operands.count(MatInstruction::T_diag_idx)) {
            auto opBin = encodeBinary(inst.diag_idx, m_formatConfig.MatWidthIdxSize);
            output.insert(output.end(), opBin.begin(), opBin.end());
        }
        if (operands.count(MatInstruction::T_elem_idx)) {
            auto opBin = encodeBinary(inst.elem_idx, m_formatConfig.MatWidthIdxSize);
            output.insert(output.end(), opBin.begin(), opBin.end());
        }
    }
    return output;
}


void MatProgram::append(const MatInstruction& inst) {
    m_instructions.push_back(inst);
}

TNPProgramBinary MatProgram::encodeBinary(MatValue_t value, size_t size) {
	TNPProgramBinary binary;
	for (size_t i = 0;i < size;i++) {
		uint8_t byte = (uint8_t)(value & 0xFF);
		binary.push_back(std::byte(byte));
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
