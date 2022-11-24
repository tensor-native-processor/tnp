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

    std::ofstream dm0("data_mem0.txt");
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) dm0 << A[i][j] << std::endl;
    dm0.close();
    std::ofstream dm1("data_mem1.txt");
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) dm1 << B[i][j] << std::endl;
    dm1.close();

    std::ofstream ans("ans.txt");
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) ans << C[i][j] << std::endl;
    ans.close();

    // Start instruction mem
    MatCoreProgram prog0, prog1;
    MatCoreInst inst;

    // Core 0

    // Load memory
    inst.opcode = MatCoreInstDefn::LOAD_MAT;
    inst.operands[MatCoreInstDefn::ADDR] = 0;
    inst.operands[MatCoreInstDefn::M1] = 5;
    prog0.append(inst);

    // set weight 5
    inst.opcode = MatCoreInstDefn::SET_WEIGHT;
    inst.operands[MatCoreInstDefn::M1] = 5;
    prog0.append(inst);

    // Send 5
    for (int i = 0;i < 16;i++) {
        inst.opcode = MatCoreInstDefn::SEND_ROW;
        inst.operands[MatCoreInstDefn::CORE_IDX] = 1;
        inst.operands[MatCoreInstDefn::M1] = 5;
        inst.operands[MatCoreInstDefn::ROW_IDX] = i;
        prog0.append(inst);
    }

    // Recv 3
    for (int i = 0;i < 16;i++) {
        inst.opcode = MatCoreInstDefn::RECV_ROW;
        inst.operands[MatCoreInstDefn::CORE_IDX] = 1;
        inst.operands[MatCoreInstDefn::M1] = 3;
        inst.operands[MatCoreInstDefn::ROW_IDX] = i;
        prog0.append(inst);
    }

    // multiply 3 to 3
    inst.opcode = MatCoreInstDefn::MULTIPLY;
    inst.operands[MatCoreInstDefn::Md] = 3;
    inst.operands[MatCoreInstDefn::M1] = 3;
    prog0.append(inst);

    // store 3 to memory
    inst.opcode = MatCoreInstDefn::STORE_MAT;
    inst.operands[MatCoreInstDefn::ADDR] = 0;
    inst.operands[MatCoreInstDefn::M1] = 3;
    prog0.append(inst);

    // halt
    inst.opcode = MatCoreInstDefn::HALT;
    prog0.append(inst);

    // Core 1

    // Load memory
    inst.opcode = MatCoreInstDefn::LOAD_MAT;
    inst.operands[MatCoreInstDefn::ADDR] = 0;
    inst.operands[MatCoreInstDefn::M1] = 2;
    prog1.append(inst);

    // Recv 0
    for (int i = 0;i < 16;i++) {
        inst.opcode = MatCoreInstDefn::RECV_ROW;
        inst.operands[MatCoreInstDefn::CORE_IDX] = 0;
        inst.operands[MatCoreInstDefn::M1] = 0;
        inst.operands[MatCoreInstDefn::ROW_IDX] = i;
        prog1.append(inst);
    }

    // Send 2
    for (int i = 0;i < 16;i++) {
        inst.opcode = MatCoreInstDefn::SEND_ROW;
        inst.operands[MatCoreInstDefn::CORE_IDX] = 0;
        inst.operands[MatCoreInstDefn::M1] = 2;
        inst.operands[MatCoreInstDefn::ROW_IDX] = i;
        prog1.append(inst);
    }

    // set weight 0
    inst.opcode = MatCoreInstDefn::SET_WEIGHT;
    inst.operands[MatCoreInstDefn::M1] = 0;
    prog1.append(inst);

    // multiply 2 to 0
    inst.opcode = MatCoreInstDefn::MULTIPLY;
    inst.operands[MatCoreInstDefn::Md] = 0;
    inst.operands[MatCoreInstDefn::M1] = 2;
    prog1.append(inst);

    // store 0 to memory
    inst.opcode = MatCoreInstDefn::STORE_MAT;
    inst.operands[MatCoreInstDefn::ADDR] = 0;
    inst.operands[MatCoreInstDefn::M1] = 0;
    prog1.append(inst);

    // halt
    inst.opcode = MatCoreInstDefn::HALT;
    prog1.append(inst);

    SaveProgram(prog0.toBinary(), "inst_mem0.txt");
    SaveProgram(prog1.toBinary(), "inst_mem1.txt");

    return 0;
}
