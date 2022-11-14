#include "mat_format.h"

// Instruction operand map
const std::map<MatInstruction::Operator, std::vector<MatInstruction::OperandTag>> MatInstruction::operandMap = {
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

std::vector<MatInstruction::OperandTag> MatInstruction::operands(Operator op) {
    return operandMap.at(op);
}
