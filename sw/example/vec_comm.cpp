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
    VecCoreInst inst;

    // Core 0

    // Load memory
    inst.opcode = VecCoreInstDefn::LOAD_VEC;
    inst.operands[VecCoreInstDefn::ADDR] = 0;
    inst.operands[VecCoreInstDefn::V1] = 5;
    prog0.append(inst);

    inst.opcode = VecCoreInstDefn::LOAD_VEC;
    inst.operands[VecCoreInstDefn::ADDR] = 16;
    inst.operands[VecCoreInstDefn::V1] = 6;
    prog0.append(inst);

    // Add two vectors
    inst.opcode = VecCoreInstDefn::ADD;
    inst.operands[VecCoreInstDefn::Vd] = 3;
    inst.operands[VecCoreInstDefn::V1] = 6;
    inst.operands[VecCoreInstDefn::V2] = 5;
    prog0.append(inst);

    // send 3 to core 1
    inst.opcode = VecCoreInstDefn::SEND_VEC;
    inst.operands[VecCoreInstDefn::CORE_IDX] = 1;
    inst.operands[VecCoreInstDefn::V1] = 3;
    prog0.append(inst);

    // receive 5 from core 1
    inst.opcode = VecCoreInstDefn::RECV_VEC;
    inst.operands[VecCoreInstDefn::CORE_IDX] = 1;
    inst.operands[VecCoreInstDefn::V1] = 5;
    prog0.append(inst);

    // store 5 to memory
    inst.opcode = VecCoreInstDefn::STORE_VEC;
    inst.operands[VecCoreInstDefn::ADDR] = 0;
    inst.operands[VecCoreInstDefn::V1] = 5;
    prog0.append(inst);

    // halt
    inst.opcode = VecCoreInstDefn::HALT;
    prog0.append(inst);

    // Core 1

    // receive 3 from core 0
    inst.opcode = VecCoreInstDefn::RECV_VEC;
    inst.operands[VecCoreInstDefn::CORE_IDX] = 0;
    inst.operands[VecCoreInstDefn::V1] = 3;
    prog1.append(inst);

    // store 3 to memory
    inst.opcode = VecCoreInstDefn::STORE_VEC;
    inst.operands[VecCoreInstDefn::ADDR] = 0;
    inst.operands[VecCoreInstDefn::V1] = 3;
    prog1.append(inst);

    // copy 3 to 7
    inst.opcode = VecCoreInstDefn::COPY;
    inst.operands[VecCoreInstDefn::Vd] = 7;
    inst.operands[VecCoreInstDefn::V1] = 3;
    prog1.append(inst);

    // send 7 to core 0
    inst.opcode = VecCoreInstDefn::SEND_VEC;
    inst.operands[VecCoreInstDefn::CORE_IDX] = 0;
    inst.operands[VecCoreInstDefn::V1] = 7;
    prog1.append(inst);

    // halt
    inst.opcode = VecCoreInstDefn::HALT;
    prog1.append(inst);

    SaveProgram(prog0.toBinary(), "inst_mem0.txt");
    SaveProgram(prog1.toBinary(), "inst_mem1.txt");

    return 0;
}
