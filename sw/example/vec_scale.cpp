#include "vec_program.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>

int main() {
    float A[16], B[16], C[16], D, E;
    for (int i = 0;i < 16;i++) A[i] = rand() % 256;
    for (int i = 0;i < 16;i++) B[i] = rand() % 256;
    D = rand() % 256;
    E = rand() % 256;
    for (int i = 0;i < 16;i++) C[i] = (A[i] + B[i]) * D + E;

    std::ofstream dm("data_mem.txt");
    // Starting at addr 0
    for (int i = 0;i < 16;i++) dm << A[i] << std::endl;
    // Starting at addr 16
    for (int i = 0;i < 16;i++) dm << B[i] << std::endl;
    dm << 0 << std::endl;
    // Starting at addr 33
    dm << D << std::endl;
    dm << E << std::endl;
    dm.close();

    std::ofstream ans("ans.txt");
    for (int i = 0;i < 16;i++) ans << C[i] << std::endl;
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

    prog.append({VecCoreInstDefn::LOAD_SCALAR, {
        {VecCoreInstDefn::ADDR, 33},
        {VecCoreInstDefn::V1, 0},
        {VecCoreInstDefn::VEC_IDX, 7}
    }});
    prog.append({VecCoreInstDefn::LOAD_SCALAR, {
        {VecCoreInstDefn::ADDR, 34},
        {VecCoreInstDefn::V1, 1},
        {VecCoreInstDefn::VEC_IDX, 2}
    }});

    // Add two vectors
    prog.append({VecCoreInstDefn::ADD, {
        {VecCoreInstDefn::Vd, 3},
        {VecCoreInstDefn::V1, 6},
        {VecCoreInstDefn::V2, 5}
    }});

    // Scale by D
    prog.append({VecCoreInstDefn::SCALE, {
        {VecCoreInstDefn::Vd, 3},
        {VecCoreInstDefn::V1, 3},
        {VecCoreInstDefn::V2, 0},
        {VecCoreInstDefn::VEC_IDX, 7}
    }});

    // Delta by E
    prog.append({VecCoreInstDefn::DELTA, {
        {VecCoreInstDefn::Vd, 3},
        {VecCoreInstDefn::V1, 3},
        {VecCoreInstDefn::V2, 1},
        {VecCoreInstDefn::VEC_IDX, 2}
    }});

    // store 3 to memory
    prog.append({VecCoreInstDefn::STORE_VEC, {
        {VecCoreInstDefn::ADDR, 0},
        {VecCoreInstDefn::V1, 3}
    }});

    // halt
    prog.append({VecCoreInstDefn::HALT, {
    }});

    TNPProgramBinary bin = prog.toBinary();
    SaveProgram(bin, "inst_mem.txt");

    return 0;
}
