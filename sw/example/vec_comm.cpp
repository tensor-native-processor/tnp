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

    std::ofstream dm0("data_mem0.txt");
    // Starting at addr 0
    for (int i = 0;i < 16;i++) dm0 << A[i] << std::endl;
    // Starting at addr 16
    for (int i = 0;i < 16;i++) dm0 << B[i] << std::endl;
    dm0.close();

    std::ofstream dm1("data_mem1.txt");
    dm1.close();

    std::ofstream ans("ans.txt");
    for (int i = 0;i < 16;i++) ans << C[i] << std::endl;
    ans.close();

    // Start instruction mem
    VecCoreProgram prog0, prog1;

    // Core 0

    // Load memory
    prog0.append({VecCoreInstDefn::LOAD_VEC, {
        {VecCoreInstDefn::ADDR, 0},
        {VecCoreInstDefn::V1, 5}
    }});

    prog0.append({VecCoreInstDefn::LOAD_VEC, {
        {VecCoreInstDefn::ADDR, 16},
        {VecCoreInstDefn::V1, 6}
    }});

    // Add two vectors
    prog0.append({VecCoreInstDefn::ADD, {
        {VecCoreInstDefn::Vd, 3},
        {VecCoreInstDefn::V1, 6},
        {VecCoreInstDefn::V2, 5}
    }});

    // send 3 to core 1
    prog0.append({VecCoreInstDefn::SEND_VEC, {
        {VecCoreInstDefn::CORE_IDX, 1},
        {VecCoreInstDefn::V1, 3}
    }});

    // receive 5 from core 1
    prog0.append({VecCoreInstDefn::RECV_VEC, {
        {VecCoreInstDefn::CORE_IDX, 1},
        {VecCoreInstDefn::V1, 5}
    }});

    // store 5 to memory
    prog0.append({VecCoreInstDefn::STORE_VEC, {
        {VecCoreInstDefn::ADDR, 0},
        {VecCoreInstDefn::V1, 5}
    }});

    // halt
    prog0.append({VecCoreInstDefn::HALT, {
    }});

    // Core 1

    // receive 3 from core 0
    prog1.append({VecCoreInstDefn::RECV_VEC, {
        {VecCoreInstDefn::CORE_IDX, 0},
        {VecCoreInstDefn::V1, 3}
    }});

    // store 3 to memory
    prog1.append({VecCoreInstDefn::STORE_VEC, {
        {VecCoreInstDefn::ADDR, 0},
        {VecCoreInstDefn::V1, 3}
    }});

    // copy 3 to 7
    prog1.append({VecCoreInstDefn::COPY, {
        {VecCoreInstDefn::Vd, 7},
        {VecCoreInstDefn::V1, 3}
    }});

    // send 7 to core 0
    prog1.append({VecCoreInstDefn::SEND_VEC, {
        {VecCoreInstDefn::CORE_IDX, 0},
        {VecCoreInstDefn::V1, 7}
    }});

    // halt
    prog1.append({VecCoreInstDefn::HALT, {
    }});

    SaveProgram(prog0.toBinary(), "inst_mem0.txt");
    SaveProgram(prog1.toBinary(), "inst_mem1.txt");

    return 0;
}
