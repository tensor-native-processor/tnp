#include "mat_program.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>

int main() {
    float A[16], B[16], C[16];
    for (int i = 0;i < 16;i++) A[i] = rand() % 256;
    for (int i = 0;i < 16;i++) B[i] = rand() % 256;
    for (int i = 0;i < 16;i++) C[i] = A[i] + B[i];

    std::ofstream dm("data_mem.txt");
    // Starting at addr 0
    for (int i = 0;i < 16;i++) dm << A[i] << std::endl;
    // Starting at addr 16
    for (int i = 0;i < 16;i++) dm << B[i] << std::endl;
    dm.close();

    std::ofstream ans("ans.txt");
    for (int i = 0;i < 16;i++) ans << C[i] << std::endl;
    ans.close();

    // Start instruction mem
    MatCoreProgram prog;

    // Load memory
    prog.append({MatCoreInstDefn::LOAD_ROW, {
        {MatCoreInstDefn::ADDR, 0},
        {MatCoreInstDefn::M1, 5},
        {MatCoreInstDefn::ROW_IDX, 9}
    }});
    prog.append({MatCoreInstDefn::LOAD_ROW, {
        {MatCoreInstDefn::ADDR, 16},
        {MatCoreInstDefn::M1, 6},
        {MatCoreInstDefn::ROW_IDX, 2}
    }});

    // Add two vectors
    prog.append({MatCoreInstDefn::ADD_ROW, {
        {MatCoreInstDefn::Md, 3},
        {MatCoreInstDefn::M1, 6},
        {MatCoreInstDefn::M2, 5},
        {MatCoreInstDefn::ROW_IDX, 0},
        {MatCoreInstDefn::ROW_IDX_1, 2},
        {MatCoreInstDefn::ROW_IDX_2, 9}
    }});

    // store 3 to memory
    prog.append({MatCoreInstDefn::STORE_ROW, {
        {MatCoreInstDefn::ADDR, 0},
        {MatCoreInstDefn::M1, 3},
        {MatCoreInstDefn::ROW_IDX, 0}
    }});

    // halt
    prog.append({MatCoreInstDefn::HALT, {
    }});

    TNPProgramBinary bin = prog.toBinary();
    SaveProgram(bin, "inst_mem.txt");

    return 0;
}
