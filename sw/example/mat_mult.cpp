#include "mat_program.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>

int main() {
    float A[16][16], B[16][16], C[16][16];
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) A[i][j] = rand() % 256;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) B[i][j] = rand() % 256;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) C[i][j] = 0;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) for (int k = 0;k < 16;k++) {
        C[i][j] += A[k][15 - i] * B[k][j];
    }

    std::ofstream dm("data_mem.txt");
    // Starting at addr 0
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) dm << A[i][j] << std::endl;
    // Starting at addr 256
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) dm << B[i][j] << std::endl;
    dm.close();

    std::ofstream ans("ans.txt");
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) ans << C[i][j] << std::endl;
    ans.close();

    // Start instruction mem
    MatCoreProgram prog;
    MatCoreInst inst;

    // Load memory
    inst.opcode = MatCoreInstDefn::LOAD_MAT;
    inst.operands[MatCoreInstDefn::ADDR] = 0;
    inst.operands[MatCoreInstDefn::M1] = 0;
    prog.append(inst);

    inst.opcode = MatCoreInstDefn::LOAD_MAT;
    inst.operands[MatCoreInstDefn::ADDR] = 256;
    inst.operands[MatCoreInstDefn::M1] = 1;
    prog.append(inst);

    // set weight 0
    inst.opcode = MatCoreInstDefn::SET_WEIGHT;
    inst.operands[MatCoreInstDefn::M1] = 0;
    prog.append(inst);

    // multiply 1 to 2
    inst.opcode = MatCoreInstDefn::MULTIPLY;
    inst.operands[MatCoreInstDefn::Md] = 2;
    inst.operands[MatCoreInstDefn::M1] = 1;
    prog.append(inst);

    // store 2 to memory
    inst.opcode = MatCoreInstDefn::STORE_MAT;
    inst.operands[MatCoreInstDefn::ADDR] = 0;
    inst.operands[MatCoreInstDefn::M1] = 2;
    prog.append(inst);

    // halt
    inst.opcode = MatCoreInstDefn::HALT;
    prog.append(inst);

    TNPProgramBinary bin = prog.toBinary();
    SaveProgram(bin, "inst_mem.txt");

    return 0;
}
