#include "vec_program.h"
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
    VecCoreProgram prog;

    // Load memory
    prog.append({
        VecCoreInstDefn::LOAD_VEC, {
            {VecCoreInstDefn::ADDR, 0},
            {VecCoreInstDefn::V1, 5}
        }
    });
    prog.append({
        VecCoreInstDefn::LOAD_VEC, {
            {VecCoreInstDefn::ADDR, 16},
            {VecCoreInstDefn::V1, 6}
        }
    });

    // Add two vectors
    prog.append({
        VecCoreInstDefn::ADD, {
            {VecCoreInstDefn::Vd, 3},
            {VecCoreInstDefn::V1, 6},
            {VecCoreInstDefn::V2, 5}
        }
    });

    // store 3 to memory
    prog.append({
        VecCoreInstDefn::STORE_VEC, {
            {VecCoreInstDefn::ADDR, 0},
            {VecCoreInstDefn::V1, 3}
        }
    });

    // halt
    prog.append({
        VecCoreInstDefn::HALT, {}
    });

    TNPProgramBinary bin = prog.toBinary();
    SaveProgram(bin, "inst_mem.txt");

    return 0;
}
