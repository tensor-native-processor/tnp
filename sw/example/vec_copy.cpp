#include "vec_program.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>

int main() {
    float A[16], B[16], C[16], D[16];
    for (int i = 0;i < 16;i++) A[i] = rand() % 256;
    for (int i = 0;i < 16;i++) B[i] = rand() % 256;
    for (int i = 0;i < 16;i++) C[i] = -A[i];
    for (int i = 0;i < 16;i++) D[i] = -A[i] * B[i];

    std::ofstream dm("data_mem.txt");
    // Starting at addr 0
    for (int i = 0;i < 16;i++) dm << A[i] << std::endl;
    // Starting at addr 16
    for (int i = 0;i < 16;i++) dm << B[i] << std::endl;
    dm.close();

    std::ofstream ans("ans.txt");
    for (int i = 0;i < 16;i++) ans << C[i] << std::endl;
    for (int i = 0;i < 16;i++) ans << D[i] << std::endl;
    ans.close();

    // Start instruction mem
    VecCoreProgram prog;
    VecCoreInst inst;

    // Load memory
    inst.opcode = VecCoreInstDefn::LOAD_VEC;
    inst.operands[VecCoreInstDefn::ADDR] = 0;
    inst.operands[VecCoreInstDefn::V1] = 5;
    prog.append(inst);

    inst.opcode = VecCoreInstDefn::LOAD_VEC;
    inst.operands[VecCoreInstDefn::ADDR] = 16;
    inst.operands[VecCoreInstDefn::V1] = 6;
    prog.append(inst);

    // Clear 1
    inst.opcode = VecCoreInstDefn::CLEAR;
    inst.operands[VecCoreInstDefn::V1] = 1;
    prog.append(inst);

    // Subtract 5 from 1, store into 1
    inst.opcode = VecCoreInstDefn::SUB;
    inst.operands[VecCoreInstDefn::Vd] = 1;
    inst.operands[VecCoreInstDefn::V1] = 1;
    inst.operands[VecCoreInstDefn::V2] = 5;
    prog.append(inst);

    // Copy 1 to 2
    inst.opcode = VecCoreInstDefn::COPY;
    inst.operands[VecCoreInstDefn::Vd] = 2;
    inst.operands[VecCoreInstDefn::V1] = 1;
    prog.append(inst);

    // Dot 2, 6 to 3
    inst.opcode = VecCoreInstDefn::DOT;
    inst.operands[VecCoreInstDefn::Vd] = 3;
    inst.operands[VecCoreInstDefn::V1] = 6;
    inst.operands[VecCoreInstDefn::V2] = 2;
    prog.append(inst);

    // Store 1
    inst.opcode = VecCoreInstDefn::STORE_VEC;
    inst.operands[VecCoreInstDefn::ADDR] = 0;
    inst.operands[VecCoreInstDefn::V1] = 1;
    prog.append(inst);

    // Store 3
    inst.opcode = VecCoreInstDefn::STORE_VEC;
    inst.operands[VecCoreInstDefn::ADDR] = 16;
    inst.operands[VecCoreInstDefn::V1] = 3;
    prog.append(inst);

    // halt
    inst.opcode = VecCoreInstDefn::HALT;
    prog.append(inst);

    TNPProgramBinary bin = prog.toBinary();
    SaveProgram(bin, "inst_mem.txt");

    return 0;
}
