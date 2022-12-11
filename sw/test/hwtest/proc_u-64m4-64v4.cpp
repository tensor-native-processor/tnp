#include "mat_program.h"
#include "vec_program.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>

int main() {
    float A[64][64];
    for (int i = 0;i < 64;i++) for (int j = 0;j < 64;j++) A[i][j] = rand() % 65536;

    std::ofstream dm0("data_mem0.txt");
    for (int i = 0;i < 64;i++) for (int j = 0;j < 64;j++) dm0 << A[i][j] << std::endl;
    dm0.close();

    for (int i = 1;i <= 7;i++) {
        std::ofstream dm("data_mem" + std::to_string(i) + ".txt");
        dm.close();
    }

    std::ofstream ans("ans.txt");
    for (int i = 0;i < 64;i++) ans << A[4][i] << std::endl;
    ans.close();

    // Start instruction mem
    MatCoreProgram prog0;
    MatCoreInst minst;
    VecCoreProgram prog7;
    VecCoreInst vinst;

    // Core 0
    // Load memory
    minst.opcode = MatCoreInstDefn::LOAD_MAT;
    minst.operands[MatCoreInstDefn::ADDR] = 0;
    minst.operands[MatCoreInstDefn::M1] = 5;
    prog0.append(minst);

    // Send 5(4) to core 7
    minst.opcode = MatCoreInstDefn::SEND_ROW;
    minst.operands[MatCoreInstDefn::CORE_IDX] = 7;
    minst.operands[MatCoreInstDefn::M1] = 5;
    minst.operands[MatCoreInstDefn::ROW_IDX] = 4;
    prog0.append(minst);

    // halt
    minst.opcode = MatCoreInstDefn::HALT;
    prog0.append(minst);

    // Core 7
    // Recv core 0 to 9
    vinst.opcode = VecCoreInstDefn::RECV_VEC;
    vinst.operands[VecCoreInstDefn::CORE_IDX] = 0;
    vinst.operands[VecCoreInstDefn::V1] = 9;
    prog7.append(vinst);

    // store 9 to memory
    vinst.opcode = VecCoreInstDefn::STORE_VEC;
    vinst.operands[VecCoreInstDefn::ADDR] = 0;
    vinst.operands[VecCoreInstDefn::V1] = 9;
    prog7.append(vinst);

    // halt
    vinst.opcode = VecCoreInstDefn::HALT;
    prog7.append(vinst);

    SaveProgram(prog0.toBinary(), "inst_mem0.txt");
    SaveProgram(prog7.toBinary(), "inst_mem7.txt");

    for (int i = 1;i <= 3;i++) {
        MatCoreProgram prog;
        MatCoreInst inst;
        inst.opcode = MatCoreInstDefn::HALT;
        prog.append(inst);
        SaveProgram(prog.toBinary(), "inst_mem" + std::to_string(i) + ".txt");
    }
    for (int i = 4;i <= 6;i++) {
        VecCoreProgram prog;
        VecCoreInst inst;
        inst.opcode = VecCoreInstDefn::HALT;
        prog.append(inst);
        SaveProgram(prog.toBinary(), "inst_mem" + std::to_string(i) + ".txt");
    }

    return 0;
}
