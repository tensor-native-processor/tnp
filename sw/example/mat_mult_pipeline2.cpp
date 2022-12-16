#include "mat_program.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>

int main() {
    float A[16][16], B[16][16], C[16][16], D[16][16], E[16][16];
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) A[i][j] = rand() % 256;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) B[i][j] = rand() % 256;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) D[i][j] = rand() % 256;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) C[i][j] = 0;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) for (int k = 0;k < 16;k++) {
        C[i][j] += A[i][k] * B[k][j];
    }
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) E[i][j] = 0;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) for (int k = 0;k < 16;k++) {
        E[i][j] += A[i][k] * D[k][j];
    }

    std::ofstream dm("data_mem.txt");
    // Starting at addr 0
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) dm << A[i][j] << std::endl;
    // Starting at addr 256
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) dm << B[i][j] << std::endl;
    // Starting at addr 512
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) dm << D[i][j] << std::endl;
    dm.close();

    std::ofstream ans("ans.txt");
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) ans << C[i][j] << std::endl;
    for (int i = 0;i < 16;i++) for (int j = 0;j < 16;j++) ans << E[i][j] << std::endl;
    ans.close();

    // Start instruction mem
    MatCoreProgram prog;

    // Load memory
    prog.append({MatCoreInstDefn::LOAD_MAT, {
        {MatCoreInstDefn::ADDR, 0},
        {MatCoreInstDefn::M1, 1}
    }});
    prog.append({MatCoreInstDefn::LOAD_MAT, {
        {MatCoreInstDefn::ADDR, 256},
        {MatCoreInstDefn::M1, 6}
    }});
    prog.append({MatCoreInstDefn::LOAD_MAT, {
        {MatCoreInstDefn::ADDR, 512},
        {MatCoreInstDefn::M1, 7}
    }});

    // xflip 1
    prog.append({MatCoreInstDefn::XFLIP, {
        {MatCoreInstDefn::M1, 1}
    }});

    // transpose 1
    prog.append({MatCoreInstDefn::TRANSPOSE, {
        {MatCoreInstDefn::M1, 1}
    }});

    // set weight 1
    prog.append({MatCoreInstDefn::SET_WEIGHT, {
        {MatCoreInstDefn::M1, 1}
    }});

    // multiply 6 to 4
    prog.append({MatCoreInstDefn::MULTIPLY, {
        {MatCoreInstDefn::Md, 4},
        {MatCoreInstDefn::M1, 6}
    }});

    // multiply 7 to 3
    prog.append({MatCoreInstDefn::MULTIPLY, {
        {MatCoreInstDefn::Md, 3},
        {MatCoreInstDefn::M1, 7}
    }});

    // store 3 to memory

    prog.append({MatCoreInstDefn::STORE_MAT, {
        {MatCoreInstDefn::ADDR, 256},
        {MatCoreInstDefn::M1, 3}
    }});
    // store 4 to memory
    prog.append({MatCoreInstDefn::STORE_MAT, {
        {MatCoreInstDefn::ADDR, 0},
        {MatCoreInstDefn::M1, 4}
    }});

    // halt
    prog.append({MatCoreInstDefn::HALT, {
    }});

    TNPProgramBinary bin = prog.toBinary();
    SaveProgram(bin, "inst_mem.txt");

    return 0;
}
