#include "mat_program.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <iomanip>

int main() {
    float A[16][16], B[16][16];
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) A[i][j] = rand() % 256;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) B[i][j] = rand() % 256;

    std::ofstream dm0("data_mem0.txt");
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) dm0 << A[i][j] << std::endl;
    dm0.close();
    std::ofstream dm1("data_mem1.txt");
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) dm1 << B[i][j] << std::endl;
    dm1.close();

    // Copy diag 15 from A to B
    for (int i = 0;i < 16;i++) B[i][15 - i] = A[i][15 - i];
    // Copy diag 3/19 from B to A
    for (int i = 0;i < 4;i++) A[i][3 - i] = B[i][3 - i];
    for (int i = 0;i < 12;i++) A[i + 4][15 - i] = B[i + 4][15 - i];
    // Copy diag 6 from A to B
    for (int i = 0;i < 7;i++) B[i][6 - i] = A[i][6 - i];
    // Copy diag 28 from B to A
    for (int i = 0;i < 3;i++) A[i + 13][15 - i] = B[i + 13][15 - i];

    std::ofstream ans0("ans0.txt");
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) ans0 << std::fixed << std::setprecision(6) << A[i][j] << std::endl;
    ans0.close();
    std::ofstream ans1("ans1.txt");
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) ans1 << std::fixed << std::setprecision(6) << B[i][j] << std::endl;
    ans1.close();

    // Start instruction mem
    MatCoreProgram prog0, prog1;
    MatCoreInst inst;

    // Core 0

    // Load memory
    inst.opcode = MatCoreInstDefn::LOAD_MAT;
    inst.operands[MatCoreInstDefn::ADDR] = 0;
    inst.operands[MatCoreInstDefn::M1] = 5;
    prog0.append(inst);

    // Send 5 diag 15
    inst.opcode = MatCoreInstDefn::SEND_DIAG;
    inst.operands[MatCoreInstDefn::CORE_IDX] = 1;
    inst.operands[MatCoreInstDefn::M1] = 5;
    inst.operands[MatCoreInstDefn::M2] = 0;
    inst.operands[MatCoreInstDefn::DIAG_IDX] = 15;
    prog0.append(inst);
    // Recv 5 diag 3
    inst.opcode = MatCoreInstDefn::RECV_DIAG;
    inst.operands[MatCoreInstDefn::CORE_IDX] = 1;
    inst.operands[MatCoreInstDefn::M1] = 5;
    inst.operands[MatCoreInstDefn::M2] = 5;
    inst.operands[MatCoreInstDefn::DIAG_IDX] = 3;
    prog0.append(inst);
    // Send 5 diag 6
    inst.opcode = MatCoreInstDefn::SEND_DIAG;
    inst.operands[MatCoreInstDefn::CORE_IDX] = 1;
    inst.operands[MatCoreInstDefn::M1] = 5;
    inst.operands[MatCoreInstDefn::M2] = 0;
    inst.operands[MatCoreInstDefn::DIAG_IDX] = 6;
    prog0.append(inst);
    // Recv 5 diag 12
    inst.opcode = MatCoreInstDefn::RECV_DIAG2;
    inst.operands[MatCoreInstDefn::CORE_IDX] = 1;
    inst.operands[MatCoreInstDefn::M1] = 5;
    inst.operands[MatCoreInstDefn::M2] = 5;
    inst.operands[MatCoreInstDefn::DIAG_IDX] = 12;
    prog0.append(inst);

    // store 5 to memory
    inst.opcode = MatCoreInstDefn::STORE_MAT;
    inst.operands[MatCoreInstDefn::ADDR] = 0;
    inst.operands[MatCoreInstDefn::M1] = 5;
    prog0.append(inst);

    // halt
    inst.opcode = MatCoreInstDefn::HALT;
    prog0.append(inst);

    // Core 1

    // Load memory
    inst.opcode = MatCoreInstDefn::LOAD_MAT;
    inst.operands[MatCoreInstDefn::ADDR] = 0;
    inst.operands[MatCoreInstDefn::M1] = 5;
    prog1.append(inst);

    // Recv 5 diag 15
    inst.opcode = MatCoreInstDefn::RECV_DIAG;
    inst.operands[MatCoreInstDefn::CORE_IDX] = 0;
    inst.operands[MatCoreInstDefn::M1] = 5;
    inst.operands[MatCoreInstDefn::M2] = 0;
    inst.operands[MatCoreInstDefn::DIAG_IDX] = 15;
    prog1.append(inst);
    // Send 5 diag 3
    inst.opcode = MatCoreInstDefn::SEND_DIAG;
    inst.operands[MatCoreInstDefn::CORE_IDX] = 0;
    inst.operands[MatCoreInstDefn::M1] = 5;
    inst.operands[MatCoreInstDefn::M2] = 5;
    inst.operands[MatCoreInstDefn::DIAG_IDX] = 3;
    prog1.append(inst);
    // Recv 5 diag 6
    inst.opcode = MatCoreInstDefn::RECV_DIAG1;
    inst.operands[MatCoreInstDefn::CORE_IDX] = 0;
    inst.operands[MatCoreInstDefn::M1] = 5;
    inst.operands[MatCoreInstDefn::M2] = 5;
    inst.operands[MatCoreInstDefn::DIAG_IDX] = 6;
    prog1.append(inst);
    // Send 5 diag 12
    inst.opcode = MatCoreInstDefn::SEND_DIAG;
    inst.operands[MatCoreInstDefn::CORE_IDX] = 0;
    inst.operands[MatCoreInstDefn::M1] = 5;
    inst.operands[MatCoreInstDefn::M2] = 5;
    inst.operands[MatCoreInstDefn::DIAG_IDX] = 12;
    prog1.append(inst);

    // store 5 to memory
    inst.opcode = MatCoreInstDefn::STORE_MAT;
    inst.operands[MatCoreInstDefn::ADDR] = 0;
    inst.operands[MatCoreInstDefn::M1] = 5;
    prog1.append(inst);

    // halt
    inst.opcode = MatCoreInstDefn::HALT;
    prog1.append(inst);

    
    SaveProgram(prog0.toBinary(), "inst_mem0.txt");
    SaveProgram(prog1.toBinary(), "inst_mem1.txt");

    return 0;
}
