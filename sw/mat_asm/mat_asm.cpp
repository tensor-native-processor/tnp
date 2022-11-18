#include "mat_program.h"
#include <cstdio>
#include <iostream>

int main() {
    MatCoreProgram prog;

    MatCoreInst inst;

    // set weight 258
    inst.opcode = MatCoreInstDefn::SET_WEIGHT;
    inst.operands[MatCoreInstDefn::M1] = 258;
    prog.append(inst);

    // load mat 12345678 5
    inst.opcode = MatCoreInstDefn::LOAD_MAT;
    inst.operands[MatCoreInstDefn::ADDR] = 12345678;
    inst.operands[MatCoreInstDefn::M1] = 5;
    prog.append(inst);

    // load scalar 998244353 2048 512 536
    inst.opcode = MatCoreInstDefn::LOAD_SCALAR;
    inst.operands[MatCoreInstDefn::ADDR] = 998244353;
    inst.operands[MatCoreInstDefn::M1] = 2048;
    inst.operands[MatCoreInstDefn::ROW_IDX] = 512;
    inst.operands[MatCoreInstDefn::COL_IDX] = 536;
    prog.append(inst);

    // multiply 23 259
    inst.opcode = MatCoreInstDefn::MULTIPLY;
    inst.operands[MatCoreInstDefn::Md] = 23;
    inst.operands[MatCoreInstDefn::M1] = 259;
    prog.append(inst);

    // store col 56 7 69
    inst.opcode = MatCoreInstDefn::STORE_COL;
    inst.operands[MatCoreInstDefn::ADDR] = 56;
    inst.operands[MatCoreInstDefn::M1] = 7;
    inst.operands[MatCoreInstDefn::COL_IDX] = 69;
    prog.append(inst);

    // halt
    inst.opcode = MatCoreInstDefn::HALT;
    prog.append(inst);

    std::cout << prog.toText() << std::endl;

    TNPProgramBinary bin = prog.toBinary();
    SaveProgram(bin, "test.txt");

    MatCoreProgram prog2;
    prog2.fromBinary(bin);
    std::cout << "Next: " << prog2.toText() << std::endl;

    return 0;
}
