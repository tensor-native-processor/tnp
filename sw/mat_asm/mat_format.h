#ifndef __MAT_FORMAT_H__
#define __MAT_FORMAT_H__

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

// Operand size definition
typedef long MatMemAddr_t;
typedef long MatRegAddr_t;
typedef long MatWidthIdx_t;
typedef long MatCoreIdx_t;

// Configure MatCore format sizes (in bytes)
struct MatFormatConfig {
    size_t MatInstSize = 1;
    size_t MatMemAddrSize = 8;
    size_t MatRegAddrSize = 2;
    size_t MatWidthIdxSize = 2;
    size_t MatCoreIdxSize = 1;
};

class MatInstruction {
public:
    // Instruction type
    enum Operator {
        // Section 1
        SET_WEIGHT      = 0b00100000,
        MULTIPLY        = 0b00100001,
        TRANSPOSE       = 0b00100010,

        // Section 2
        LOAD_MAT        = 0b01000000,
        LOAD_ROW        = 0b01000001,
        LOAD_COL        = 0b01000010,
        LOAD_SCALAR     = 0b01000011,
        STORE_MAT       = 0b01010000,
        STORE_ROW       = 0b01010001,
        STORE_COL       = 0b01010010,
        STORE_SCALAR    = 0b01010011,

        // Section 3
        SEND_ROW        = 0b01100000,
        SEND_COL        = 0b01100001,
        SEND_DIAG       = 0b01100010,
        RECV_ROW        = 0b01110000,
        RECV_COL        = 0b01110001,
        RECV_SCALAR     = 0b01110010,
        RECV_DIAG       = 0b01111000,
        RECV_DIAG1      = 0b01111001,
        RECV_DIAG2      = 0b01111010,

        // Section 4
        HALT            = 0b10000000,
    } op;
    MatMemAddr_t addr;
    MatRegAddr_t Md, M1, M2;
    MatWidthIdx_t row_idx, col_idx, diag_idx, elem_idx;
    MatCoreIdx_t core_idx;

    // Operand definition
    enum OperandTag {
        T_addr,
        T_Md, T_M1, T_M2,
        T_row_idx, T_col_idx, T_diag_idx, T_elem_idx,
        T_core_idx,
    };
    static std::vector<OperandTag> operands(Operator op);

private:
    static const std::map<Operator, std::vector<OperandTag>> operandMap;
};

class MatAssembler {
public:
    MatAssembler(): m_formatConfig() {}
    MatAssembler(const MatFormatConfig& cfg): m_formatConfig(cfg) {}

    std::vector<uint8_t> toBinary(const std::vector<MatInstruction>&);
    std::string toText(const std::vector<MatInstruction>&);

private:
    MatFormatConfig m_formatConfig;
};

#endif
