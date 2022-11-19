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

    // load row 3 5 3
    inst.opcode = MatCoreInstDefn::LOAD_ROW;
    inst.operands[MatCoreInstDefn::ADDR] = 3;
    inst.operands[MatCoreInstDefn::M1] = 5;
    inst.operands[MatCoreInstDefn::ROW_IDX] = 3;
    prog.append(inst);

    // load col 45 4 2
    inst.opcode = MatCoreInstDefn::LOAD_COL;
    inst.operands[MatCoreInstDefn::ADDR] = 45;
    inst.operands[MatCoreInstDefn::M1] = 4;
    inst.operands[MatCoreInstDefn::COL_IDX] = 2;
    prog.append(inst);

    // load scalar 67 2 8 9
    inst.opcode = MatCoreInstDefn::LOAD_SCALAR;
    inst.operands[MatCoreInstDefn::ADDR] = 67;
    inst.operands[MatCoreInstDefn::M1] = 2;
    inst.operands[MatCoreInstDefn::ROW_IDX] = 8;
    inst.operands[MatCoreInstDefn::COL_IDX] = 9;
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

    // store row 100 5 3
    inst.opcode = MatCoreInstDefn::STORE_ROW;
    inst.operands[MatCoreInstDefn::ADDR] = 100;
    inst.operands[MatCoreInstDefn::M1] = 5;
    inst.operands[MatCoreInstDefn::ROW_IDX] = 3;
    prog.append(inst);

    // store col 150 4 2
    inst.opcode = MatCoreInstDefn::STORE_COL;
    inst.operands[MatCoreInstDefn::ADDR] = 150;
    inst.operands[MatCoreInstDefn::M1] = 4;
    inst.operands[MatCoreInstDefn::COL_IDX] = 2;
    prog.append(inst);

    // store scalar 160 5 3 2
    inst.opcode = MatCoreInstDefn::STORE_SCALAR;
    inst.operands[MatCoreInstDefn::ADDR] = 160;
    inst.operands[MatCoreInstDefn::M1] = 5;
    inst.operands[MatCoreInstDefn::ROW_IDX] = 3;
    inst.operands[MatCoreInstDefn::COL_IDX] = 2;
    prog.append(inst);

    // store scalar 162 2 8 9
    inst.opcode = MatCoreInstDefn::STORE_SCALAR;
    inst.operands[MatCoreInstDefn::ADDR] = 162;
    inst.operands[MatCoreInstDefn::M1] = 2;
    inst.operands[MatCoreInstDefn::ROW_IDX] = 8;
    inst.operands[MatCoreInstDefn::COL_IDX] = 9;
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
