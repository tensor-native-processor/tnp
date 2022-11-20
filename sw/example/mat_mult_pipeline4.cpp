#include "mat_program.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>

int main() {
    float A[16][16], B[16][16], C[16][16], D[16][16], E[16][16], F[16][16], G[16][16], H[16][16];
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) A[i][j] = rand() % 256;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) B[i][j] = rand() % 256;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) D[i][j] = rand() % 256;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) F[i][j] = rand() % 256;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) G[i][j] = rand() % 256;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) C[i][j] = 0;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) for (int k = 0;k < 16;k++) {
        C[i][j] += A[i][k] * B[k][j];
    }
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) E[i][j] = 0;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) for (int k = 0;k < 16;k++) {
        E[i][j] += A[i][k] * D[k][j];
    }
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) H[i][j] = 0;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) for (int k = 0;k < 16;k++) {
        H[i][j] += F[i][k] * G[k][j];
    }

    std::ofstream dm("data_mem.txt");
    // Starting at addr 0
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) dm << A[i][j] << std::endl;
    // Starting at addr 256
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) dm << B[i][j] << std::endl;
    // Starting at addr 512
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) dm << D[i][j] << std::endl;
    // Starting at addr 768
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) dm << F[i][j] << std::endl;
    // Starting at addr 1024
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) dm << G[i][j] << std::endl;
    dm.close();

    std::ofstream ans("ans.txt");
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) ans << C[i][j] << std::endl;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) ans << E[i][j] << std::endl;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) ans << H[i][j] << std::endl;
    ans.close();

    // Start instruction mem
    MatCoreProgram prog;
    MatCoreInst inst;

    // Load memory
    for (int i = 0;i < 16;i++) {
        inst.opcode = MatCoreInstDefn::LOAD_ROW;
        inst.operands[MatCoreInstDefn::ADDR] = 16 * i;
        inst.operands[MatCoreInstDefn::M1] = 1;
        inst.operands[MatCoreInstDefn::ROW_IDX] = i;
        prog.append(inst);
    }
    for (int i = 0;i < 16;i++) {
        inst.opcode = MatCoreInstDefn::LOAD_ROW;
        inst.operands[MatCoreInstDefn::ADDR] = 256 + 16 * i;
        inst.operands[MatCoreInstDefn::M1] = 6;
        inst.operands[MatCoreInstDefn::ROW_IDX] = i;
        prog.append(inst);
    }
    for (int i = 0;i < 16;i++) {
        inst.opcode = MatCoreInstDefn::LOAD_ROW;
        inst.operands[MatCoreInstDefn::ADDR] = 512 + 16 * i;
        inst.operands[MatCoreInstDefn::M1] = 7;
        inst.operands[MatCoreInstDefn::ROW_IDX] = i;
        prog.append(inst);
    }
    for (int i = 0;i < 16;i++) {
        inst.opcode = MatCoreInstDefn::LOAD_ROW;
        inst.operands[MatCoreInstDefn::ADDR] = 768 + 16 * i;
        inst.operands[MatCoreInstDefn::M1] = 2;
        inst.operands[MatCoreInstDefn::ROW_IDX] = i;
        prog.append(inst);
    }
    for (int i = 0;i < 16;i++) {
        inst.opcode = MatCoreInstDefn::LOAD_ROW;
        inst.operands[MatCoreInstDefn::ADDR] = 1024 + 16 * i;
        inst.operands[MatCoreInstDefn::M1] = 0;
        inst.operands[MatCoreInstDefn::ROW_IDX] = i;
        prog.append(inst);
    }

    // xflip 1
    inst.opcode = MatCoreInstDefn::XFLIP;
    inst.operands[MatCoreInstDefn::M1] = 1;
    prog.append(inst);

    // transpose 1
    inst.opcode = MatCoreInstDefn::TRANSPOSE;
    inst.operands[MatCoreInstDefn::M1] = 1;
    prog.append(inst);

    // xflip 2
    inst.opcode = MatCoreInstDefn::XFLIP;
    inst.operands[MatCoreInstDefn::M1] = 2;
    prog.append(inst);

    // transpose 2
    inst.opcode = MatCoreInstDefn::TRANSPOSE;
    inst.operands[MatCoreInstDefn::M1] = 2;
    prog.append(inst);

    // set weight 1
    inst.opcode = MatCoreInstDefn::SET_WEIGHT;
    inst.operands[MatCoreInstDefn::M1] = 1;
    prog.append(inst);

    // multiply 6 to 4
    inst.opcode = MatCoreInstDefn::MULTIPLY;
    inst.operands[MatCoreInstDefn::Md] = 4;
    inst.operands[MatCoreInstDefn::M1] = 6;
    prog.append(inst);

    // multiply 7 to 3
    inst.opcode = MatCoreInstDefn::MULTIPLY;
    inst.operands[MatCoreInstDefn::Md] = 3;
    inst.operands[MatCoreInstDefn::M1] = 7;
    prog.append(inst);

    // set weight 2
    inst.opcode = MatCoreInstDefn::SET_WEIGHT;
    inst.operands[MatCoreInstDefn::M1] = 2;
    prog.append(inst);

    // multiply 0 to 5
    inst.opcode = MatCoreInstDefn::MULTIPLY;
    inst.operands[MatCoreInstDefn::Md] = 5;
    inst.operands[MatCoreInstDefn::M1] = 0;
    prog.append(inst);

    // store 3 to memory
    for (int i = 0;i < 16;i++) {
        inst.opcode = MatCoreInstDefn::STORE_ROW;
        inst.operands[MatCoreInstDefn::ADDR] = 256 + 16 * i;
        inst.operands[MatCoreInstDefn::M1] = 3;
        inst.operands[MatCoreInstDefn::ROW_IDX] = i;
        prog.append(inst);
    }
    // store 4 to memory
    for (int i = 0;i < 16;i++) {
        inst.opcode = MatCoreInstDefn::STORE_ROW;
        inst.operands[MatCoreInstDefn::ADDR] = 16 * i;
        inst.operands[MatCoreInstDefn::M1] = 4;
        inst.operands[MatCoreInstDefn::ROW_IDX] = i;
        prog.append(inst);
    }
    // store 5 to memory
    for (int i = 0;i < 16;i++) {
        inst.opcode = MatCoreInstDefn::STORE_ROW;
        inst.operands[MatCoreInstDefn::ADDR] = 512 + 16 * i;
        inst.operands[MatCoreInstDefn::M1] = 5;
        inst.operands[MatCoreInstDefn::ROW_IDX] = i;
        prog.append(inst);
    }

    // halt
    inst.opcode = MatCoreInstDefn::HALT;
    prog.append(inst);

    TNPProgramBinary bin = prog.toBinary();
    SaveProgram(bin, "inst_mem.txt");

    return 0;
}
