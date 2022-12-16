#include "vec_program.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>

int main() {
    float A[16], B[16], C[16], D[16];
    for (int i = 0;i < 16;i++) A[i] = rand() % 256;
    for (int i = 0;i < 16;i++) B[i] = rand() % 256;
    for (int i = 0;i < 16;i++) C[i] = -A[i];
    for (int i = 0;i < 16;i++) D[i] = -A[i] * B[i];

    std::ofstream dm("data_mem.txt");
    // Starting at addr 0
    for (int i = 0;i < 16;i++) dm << A[i] << std::endl;
    // Starting at addr 16
    for (int i = 0;i < 16;i++) dm << B[i] << std::endl;
    dm.close();

    std::ofstream ans("ans.txt");
    for (int i = 0;i < 16;i++) ans << C[i] << std::endl;
    for (int i = 0;i < 16;i++) ans << D[i] << std::endl;
    ans.close();

    // Start instruction mem
    VecCoreProgram prog;

    // Load memory
    prog.append({VecCoreInstDefn::LOAD_VEC, {
        {VecCoreInstDefn::ADDR, 0},
        {VecCoreInstDefn::V1, 5}
    }});

    prog.append({VecCoreInstDefn::LOAD_VEC, {
        {VecCoreInstDefn::ADDR, 16},
        {VecCoreInstDefn::V1, 6}
    }});

    // Clear 1
    prog.append({VecCoreInstDefn::CLEAR, {
        {VecCoreInstDefn::V1, 1}
    }});

    // Subtract 5 from 1, store into 1
    prog.append({VecCoreInstDefn::SUB, {
        {VecCoreInstDefn::Vd, 1},
        {VecCoreInstDefn::V1, 1},
        {VecCoreInstDefn::V2, 5}
    }});

    // Copy 1 to 2
    prog.append({VecCoreInstDefn::COPY, {
        {VecCoreInstDefn::Vd, 2},
        {VecCoreInstDefn::V1, 1}
    }});

    // Dot 2, 6 to 3
    prog.append({VecCoreInstDefn::DOT, {
        {VecCoreInstDefn::Vd, 3},
        {VecCoreInstDefn::V1, 6},
        {VecCoreInstDefn::V2, 2}
    }});

    // Store 1
    prog.append({VecCoreInstDefn::STORE_VEC, {
        {VecCoreInstDefn::ADDR, 0},
        {VecCoreInstDefn::V1, 1}
    }});

    // Store 3
    prog.append({VecCoreInstDefn::STORE_VEC, {
        {VecCoreInstDefn::ADDR, 16},
        {VecCoreInstDefn::V1, 3}
    }});

    // halt
    prog.append({VecCoreInstDefn::HALT, {
    }});

    TNPProgramBinary bin = prog.toBinary();
    SaveProgram(bin, "inst_mem.txt");

    return 0;
}
