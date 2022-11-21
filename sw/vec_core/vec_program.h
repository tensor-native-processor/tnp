#ifndef __VEC_PROGRAM_H__
#define __VEC_PROGRAM_H__

#include "program.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

// Vector Core instruction definition
class VecCoreInstDefn {
public:
    // Instruction type
    enum Opcode {
        // Section 1
        ADD             = 0b00100000,
        SUB             = 0b00100001,
        DOT             = 0b00100010,
        SCALE           = 0b00100011,
        DELTA           = 0b00100100,
        ACT_SIGMOID     = 0b00110000,
        ACT_TANH        = 0b00110001,
        ACT_RELU        = 0b00110010,

        // Section 2
        LOAD_VEC        = 0b01000000,
        LOAD_SCALAR     = 0b01000001,
        STORE_VEC       = 0b01010000,
        STORE_SCALAR    = 0b01010001,

        // Section 3
        SEND_VEC        = 0b01100000,
        SEND_SCALAR     = 0b01100001,

        RECV_VEC        = 0b01110000,
        RECV_SCALAR     = 0b01110001,

        // Section 4
        HALT            = 0b10000000,
    };

    // Operand definition
    enum Operand {
        ADDR,
        CORE_IDX,
        Vd, V1, V2,
        VEC_IDX,
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


// Configure VecCore instruction sizes (in bytes)
struct VecCoreInstSize {
    // Size of each type
    std::map<VecCoreInstDefn::Type, size_t> size;

    // Default sizes
    VecCoreInstSize() {
        size[VecCoreInstDefn::OPCODE_TYPE] = 1;
        size[VecCoreInstDefn::MEM_ADDR_TYPE] = 8;
        size[VecCoreInstDefn::CORE_IDX_TYPE] = 1;
        size[VecCoreInstDefn::REG_ADDR_TYPE] = 2;
        size[VecCoreInstDefn::WIDTH_IDX_TYPE] = 2;
    }

    size_t& operator[](VecCoreInstDefn::Type);
    const size_t& operator[] (VecCoreInstDefn::Type) const;
};


typedef BaseCoreInst<VecCoreInstDefn> VecCoreInst;
typedef BaseCoreProgram<VecCoreInstDefn, VecCoreInstSize> VecCoreProgram;

// VecCore parameters
struct VecCoreParam {
    size_t width = 16;
};


// VecCore simulation engine
class VecCoreSimEngine {
public:
    VecCoreSimEngine(const VecCoreProgram&, const VecCoreParam&);

    void simulateStep();
    bool isDone() const;

private:
    enum class State {
        INIT, READY, NEXT, STOP,
    };
    VecCoreProgram m_prog;
    VecCoreParam m_param;

    // Internal state
    State m_state;
    size_t m_pc;
};


#endif
