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
        C[i][j] += A[i][k] * B[k][j];
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

    // Load memory
    prog.append({MatCoreInstDefn::LOAD_MAT, {
        {MatCoreInstDefn::ADDR, 0},
        {MatCoreInstDefn::M1, 0}
    }});
    prog.append({MatCoreInstDefn::LOAD_MAT, {
        {MatCoreInstDefn::ADDR, 256},
        {MatCoreInstDefn::M1, 1}
    }});

    // xflip 0
    prog.append({MatCoreInstDefn::XFLIP, {
        {MatCoreInstDefn::M1, 0}
    }});

    // transpose 0
    prog.append({MatCoreInstDefn::TRANSPOSE, {
        {MatCoreInstDefn::M1, 0}
    }});

    // set weight 0
    prog.append({MatCoreInstDefn::SET_WEIGHT, {
        {MatCoreInstDefn::M1, 0}
    }});

    // multiply 1 to 2
    prog.append({MatCoreInstDefn::MULTIPLY, {
        {MatCoreInstDefn::Md, 2},
        {MatCoreInstDefn::M1, 1}
    }});

    // store 2 to memory
    prog.append({MatCoreInstDefn::STORE_MAT, {
        {MatCoreInstDefn::ADDR, 0},
        {MatCoreInstDefn::M1, 2}
    }});

    // halt
    prog.append({MatCoreInstDefn::HALT, {
    }});

    TNPProgramBinary bin = prog.toBinary();
    SaveProgram(bin, "inst_mem.txt");

    return 0;
}
