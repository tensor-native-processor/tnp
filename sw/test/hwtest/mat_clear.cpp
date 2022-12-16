#include "mat_program.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>

int main() {
    float A[16][16], B[16][16];
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) A[i][j] = rand() % 256;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) B[i][j] = rand() % 256;

    std::ofstream dm("data_mem.txt");
    // Starting at addr 0
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) dm << A[i][j] << std::endl;
    // Starting at addr 256
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) dm << B[i][j] << std::endl;
    dm.close();

    // Start instruction mem
    MatCoreProgram prog;
    MatCoreInst inst;

    // Load memory
    inst.opcode = MatCoreInstDefn::LOAD_MAT;
    inst.operands[MatCoreInstDefn::ADDR] = 0;
    inst.operands[MatCoreInstDefn::M1] = 3;
    prog.append(inst);

    inst.opcode = MatCoreInstDefn::LOAD_MAT;
    inst.operands[MatCoreInstDefn::ADDR] = 256;
    inst.operands[MatCoreInstDefn::M1] = 1;
    prog.append(inst);

    // Copy 3 to 0
    inst.opcode = MatCoreInstDefn::COPY;
    inst.operands[MatCoreInstDefn::Md] = 0;
    inst.operands[MatCoreInstDefn::M1] = 3;
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

    // Copy 2 to 5
    inst.opcode = MatCoreInstDefn::COPY;
    inst.operands[MatCoreInstDefn::Md] = 5;
    inst.operands[MatCoreInstDefn::M1] = 2;
    prog.append(inst);

    // Clear 0 1 2 3 5
    prog.append({MatCoreInstDefn::CLEAR, {
        {MatCoreInstDefn::M1, 0}
    }});
    prog.append({MatCoreInstDefn::CLEAR, {
        {MatCoreInstDefn::M1, 1}
    }});
    prog.append({MatCoreInstDefn::CLEAR, {
        {MatCoreInstDefn::M1, 2}
    }});
    prog.append({MatCoreInstDefn::CLEAR, {
        {MatCoreInstDefn::M1, 3}
    }});
    prog.append({MatCoreInstDefn::CLEAR, {
        {MatCoreInstDefn::M1, 5}
    }});

    // store 0 1 2 3 5 to memory
    prog.append({MatCoreInstDefn::STORE_MAT, {
        {MatCoreInstDefn::ADDR, 0},
        {MatCoreInstDefn::M1, 0}
    }});
    prog.append({MatCoreInstDefn::STORE_MAT, {
        {MatCoreInstDefn::ADDR, 256},
        {MatCoreInstDefn::M1, 1}
    }});
    prog.append({MatCoreInstDefn::STORE_MAT, {
        {MatCoreInstDefn::ADDR, 512},
        {MatCoreInstDefn::M1, 2}
    }});
    prog.append({MatCoreInstDefn::STORE_MAT, {
        {MatCoreInstDefn::ADDR, 768},
        {MatCoreInstDefn::M1, 3}
    }});
    prog.append({MatCoreInstDefn::STORE_MAT, {
        {MatCoreInstDefn::ADDR, 1024},
        {MatCoreInstDefn::M1, 5}
    }});

    // halt
    inst.opcode = MatCoreInstDefn::HALT;
    prog.append({MatCoreInstDefn::HALT, {
    }});

    TNPProgramBinary bin = prog.toBinary();
    SaveProgram(bin, "inst_mem.txt");

    return 0;
}
