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

// Configure MatCore format sizes (in bytes)
struct MatFormatConfig {
    size_t MatInstSize = 1;
    size_t MatMemAddrSize = 8;
    size_t MatCoreIdxSize = 1;
    size_t MatRegAddrSize = 2;
    size_t MatWidthIdxSize = 2;
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
    } opcode;

    // Operand definition
    enum Operand {
        ADDR,
        CORE_IDX,
        Md, M1, M2,
        ROW_IDX, COL_IDX, DIAG_IDX, ELEM_IDX,
    };
    std::map<Operand, MatValue_t> operands;

    static std::vector<Operand> getOpcodeOperands(Opcode);
    static std::string getOpcodeName(Opcode);
    static Opcode findOpcodeByName(std::string);

private:
    static const std::map<Opcode, std::vector<Operand>> operandMap;
    static const std::map<Opcode, std::string> opcodeName;
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
