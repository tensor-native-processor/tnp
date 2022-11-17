#ifndef __MAT_FORMAT_H__
#define __MAT_FORMAT_H__

#include "program.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

// Operand size definition
typedef unsigned long MatValue_t;
typedef unsigned long MatMemAddr_t;
typedef unsigned long MatRegAddr_t;
typedef unsigned long MatWidthIdx_t;
typedef unsigned long MatCoreIdx_t;

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
    enum Opcode {
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

        // Other
        ILLEGAL_OP      = 0b11111111,
    } op;
    MatMemAddr_t addr;
    MatRegAddr_t Md, M1, M2;
    MatWidthIdx_t row_idx, col_idx, diag_idx, elem_idx;
    MatCoreIdx_t core_idx;

    // Operand definition
    enum OperandTag {
        T_addr,
        T_core_idx,
        T_Md, T_M1, T_M2,
        T_row_idx, T_col_idx, T_diag_idx, T_elem_idx,
    };
    static std::set<OperandTag> operands(Opcode);
    static std::string getOpcodeName(Opcode);
    static Opcode findOpcodeByName(std::string);

private:
    static const std::map<Opcode, std::set<OperandTag>> operandMap;
    static const std::map<Opcode, std::string> operatorName;
};

class MatProgram {
public:
    MatProgram(): m_formatConfig() {}
    MatProgram(const MatFormatConfig& cfg): m_formatConfig(cfg) {}

    // Binary is in little-endian
    TNPProgramBinary toBinary() const;
    std::string toText() const;
    void fromBinary(const TNPProgramBinary&);
    void fromText(const std::string&);

    void append(const MatInstruction&);

private:
    MatFormatConfig m_formatConfig;
    std::vector<MatInstruction> m_instructions;

	static TNPProgramBinary encodeBinary(MatValue_t, size_t);
	static MatValue_t decodeBinary(const TNPProgramBinary&);
};

#endif
