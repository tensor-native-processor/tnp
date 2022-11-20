#ifndef __MAT_FORMAT_H__
#define __MAT_FORMAT_H__

#include "program.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

// Matrix Core instruction definition
class MatCoreInstDefn {
public:
    // Instruction type
    enum Opcode {
        // Section 1
        SET_WEIGHT      = 0b00100000,
        MULTIPLY        = 0b00100001,
        TRANSPOSE       = 0b00100010,
        XFLIP           = 0b00100100,
        YFLIP           = 0b00100101,

        // Section 2
        LOAD_ROW        = 0b01000001,
        LOAD_COL        = 0b01000010,
        LOAD_SCALAR     = 0b01000011,
        STORE_ROW       = 0b01010001,
        STORE_COL       = 0b01010010,
        STORE_SCALAR    = 0b01010011,

        // Section 3
        SEND_ROW        = 0b01100000,
        SEND_COL        = 0b01100001,
        SEND_SCALAR     = 0b01100010,
        SEND_DIAG       = 0b01101000,

        RECV_ROW        = 0b01110000,
        RECV_COL        = 0b01110001,
        RECV_SCALAR     = 0b01110010,
        RECV_DIAG       = 0b01111000,
        RECV_DIAG1      = 0b01111001,
        RECV_DIAG2      = 0b01111010,

        // Section 4
        HALT            = 0b10000000,
    };

    // Operand definition
    enum Operand {
        ADDR,
        CORE_IDX,
        Md, M1, M2,
        ROW_IDX, COL_IDX, DIAG_IDX,
    };

    // Types
    enum Type {
        OPCODE_TYPE,
        MEM_ADDR_TYPE,
        CORE_IDX_TYPE,
        REG_ADDR_TYPE,
        WIDTH_IDX_TYPE,
    };

    static std::vector<Operand> getOpcodeOperands(Opcode);
    static std::string getOpcodeName(Opcode);
    static Opcode findOpcodeByName(std::string);
    static Type getOperandType(Operand);

private:
    static const std::map<Opcode, std::vector<Operand>> operandMap;
    static const std::map<Opcode, std::string> opcodeName;
    static const std::map<Operand, Type> operandType;
};


// Configure MatCore instruction sizes (in bytes)
struct MatCoreInstSize {
    // Size of each type
    std::map<MatCoreInstDefn::Type, size_t> size;

    // Default sizes
    MatCoreInstSize() {
        size[MatCoreInstDefn::OPCODE_TYPE] = 1;
        size[MatCoreInstDefn::MEM_ADDR_TYPE] = 8;
        size[MatCoreInstDefn::CORE_IDX_TYPE] = 1;
        size[MatCoreInstDefn::REG_ADDR_TYPE] = 2;
        size[MatCoreInstDefn::WIDTH_IDX_TYPE] = 2;
    }

    size_t& operator[](MatCoreInstDefn::Type);
    const size_t& operator[] (MatCoreInstDefn::Type) const;
};


typedef BaseCoreInst<MatCoreInstDefn> MatCoreInst;
typedef BaseCoreProgram<MatCoreInstDefn, MatCoreInstSize> MatCoreProgram;

// MatCore parameters
struct MatCoreParam {
    size_t width = 16;
};


// MatCore simulation engine
class MatCoreSimEngine {
public:
    MatCoreSimEngine(const MatCoreProgram&, const MatCoreParam&);

    void simulateStep();
    bool isDone() const;

private:
    enum class State {
        INIT, READY, NEXT, STOP,
        P0XX, P01X, P012,
        PX0X, PX01,
        PXX0
    };
    MatCoreProgram m_prog;
    MatCoreParam m_param;

    // Internal state
    State m_state;
    size_t m_pc;
    size_t m_diag_progress_counter;
};


#endif
