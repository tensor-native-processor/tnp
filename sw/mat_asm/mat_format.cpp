#include "mat_format.h"
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
};

std::set<MatInstruction::OperandTag> MatInstruction::operands(Operator op) {
    return operandMap.at(op);
}

// Convert MatProgram to/from text
std::string MatProgram::toText() const {
    std::ostringstream oss;
    for (auto const &inst : m_instructions) {
        oss << (int)(inst.op) << " ";
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
    int op;
    while (iss >> op) {
        inst.op = (MatInstruction::Operator)(op);
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

void MatProgram::append(const MatInstruction& inst) {
    m_instructions.push_back(inst);
}
