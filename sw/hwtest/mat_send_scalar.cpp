#include "mat_program.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>

int main() {
    float A[16][16], B[16][16], C[16][16];
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) A[i][j] = B[i][j] = 0;

    A[4][7] = 23;
    A[1][1] = 9;
    B[6][2] = 2;
    B[5][8] = 12;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) C[i][j] = A[i][j] + B[i][j];

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

    // Send 5 (4, 7)
    inst.opcode = MatCoreInstDefn::SEND_SCALAR;
    inst.operands[MatCoreInstDefn::CORE_IDX] = 1;
    inst.operands[MatCoreInstDefn::M1] = 5;
    inst.operands[MatCoreInstDefn::ROW_IDX] = 4;
    inst.operands[MatCoreInstDefn::COL_IDX] = 7;
    prog0.append(inst);
    // Send 5 (1, 1)
    inst.opcode = MatCoreInstDefn::SEND_SCALAR;
    inst.operands[MatCoreInstDefn::CORE_IDX] = 1;
    inst.operands[MatCoreInstDefn::M1] = 5;
    inst.operands[MatCoreInstDefn::ROW_IDX] = 1;
    inst.operands[MatCoreInstDefn::COL_IDX] = 1;
    prog0.append(inst);

    // Recv 5 (6, 2)
    inst.opcode = MatCoreInstDefn::RECV_SCALAR;
    inst.operands[MatCoreInstDefn::CORE_IDX] = 1;
    inst.operands[MatCoreInstDefn::M1] = 5;
    inst.operands[MatCoreInstDefn::ROW_IDX] = 6;
    inst.operands[MatCoreInstDefn::COL_IDX] = 2;
    prog0.append(inst);
    // Recv 5 (5, 8)
    inst.opcode = MatCoreInstDefn::RECV_SCALAR;
    inst.operands[MatCoreInstDefn::CORE_IDX] = 1;
    inst.operands[MatCoreInstDefn::M1] = 5;
    inst.operands[MatCoreInstDefn::ROW_IDX] = 5;
    inst.operands[MatCoreInstDefn::COL_IDX] = 8;
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

    // Recv 5 (4, 7)
    inst.opcode = MatCoreInstDefn::RECV_SCALAR;
    inst.operands[MatCoreInstDefn::CORE_IDX] = 0;
    inst.operands[MatCoreInstDefn::M1] = 5;
    inst.operands[MatCoreInstDefn::ROW_IDX] = 4;
    inst.operands[MatCoreInstDefn::COL_IDX] = 7;
    prog1.append(inst);
    // Recv 5 (1, 1)
    inst.opcode = MatCoreInstDefn::RECV_SCALAR;
    inst.operands[MatCoreInstDefn::CORE_IDX] = 0;
    inst.operands[MatCoreInstDefn::M1] = 5;
    inst.operands[MatCoreInstDefn::ROW_IDX] = 1;
    inst.operands[MatCoreInstDefn::COL_IDX] = 1;
    prog1.append(inst);

    // Send 5 (6, 2)
    inst.opcode = MatCoreInstDefn::SEND_SCALAR;
    inst.operands[MatCoreInstDefn::CORE_IDX] = 0;
    inst.operands[MatCoreInstDefn::M1] = 5;
    inst.operands[MatCoreInstDefn::ROW_IDX] = 6;
    inst.operands[MatCoreInstDefn::COL_IDX] = 2;
    prog1.append(inst);
    // Send 5 (5, 8)
    inst.opcode = MatCoreInstDefn::SEND_SCALAR;
    inst.operands[MatCoreInstDefn::CORE_IDX] = 0;
    inst.operands[MatCoreInstDefn::M1] = 5;
    inst.operands[MatCoreInstDefn::ROW_IDX] = 5;
    inst.operands[MatCoreInstDefn::COL_IDX] = 8;
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
