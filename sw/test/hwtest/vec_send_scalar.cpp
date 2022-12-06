#include "vec_program.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>

int main() {
    float A[16], B[16];
    for (int i = 0;i < 16;i++) A[i] = 0;
    for (int i = 0;i < 16;i++) B[i] = 0;
    A[3] = 9;
    A[4] = 6;
    B[7] = 2;
    B[9] = 7;

    std::ofstream dm0("data_mem0.txt");
    for (int i = 0;i < 16;i++) dm0 << A[i] << std::endl;
    dm0.close();

    std::ofstream dm1("data_mem1.txt");
    for (int i = 0;i < 16;i++) dm1 << B[i] << std::endl;
    dm1.close();

    // Start instruction mem
    VecCoreProgram prog0, prog1;
    VecCoreInst inst;

    // Core 0

    // Load memory
    inst.opcode = VecCoreInstDefn::LOAD_VEC;
    inst.operands[VecCoreInstDefn::ADDR] = 0;
    inst.operands[VecCoreInstDefn::V1] = 1;
    prog0.append(inst);

    // send 3 to core 1
    inst.opcode = VecCoreInstDefn::SEND_SCALAR;
    inst.operands[VecCoreInstDefn::CORE_IDX] = 1;
    inst.operands[VecCoreInstDefn::V1] = 1;
    inst.operands[VecCoreInstDefn::VEC_IDX] = 3;
    prog0.append(inst);
    // send 4 to core 1
    inst.opcode = VecCoreInstDefn::SEND_SCALAR;
    inst.operands[VecCoreInstDefn::CORE_IDX] = 1;
    inst.operands[VecCoreInstDefn::V1] = 1;
    inst.operands[VecCoreInstDefn::VEC_IDX] = 4;
    prog0.append(inst);

    // receive 3 from core 1
    inst.opcode = VecCoreInstDefn::RECV_SCALAR;
    inst.operands[VecCoreInstDefn::CORE_IDX] = 1;
    inst.operands[VecCoreInstDefn::V1] = 1;
    inst.operands[VecCoreInstDefn::VEC_IDX] = 3;
    prog0.append(inst);
    // receive 4 from core 1
    inst.opcode = VecCoreInstDefn::RECV_SCALAR;
    inst.operands[VecCoreInstDefn::CORE_IDX] = 1;
    inst.operands[VecCoreInstDefn::V1] = 1;
    inst.operands[VecCoreInstDefn::VEC_IDX] = 4;
    prog0.append(inst);

    // store to memory
    inst.opcode = VecCoreInstDefn::STORE_VEC;
    inst.operands[VecCoreInstDefn::ADDR] = 0;
    inst.operands[VecCoreInstDefn::V1] = 1;
    prog0.append(inst);

    // halt
    inst.opcode = VecCoreInstDefn::HALT;
    prog0.append(inst);

    // Core 1

    // Load memory
    inst.opcode = VecCoreInstDefn::LOAD_VEC;
    inst.operands[VecCoreInstDefn::ADDR] = 0;
    inst.operands[VecCoreInstDefn::V1] = 2;
    prog1.append(inst);

    // receive 3 from core 0
    inst.opcode = VecCoreInstDefn::RECV_SCALAR;
    inst.operands[VecCoreInstDefn::CORE_IDX] = 0;
    inst.operands[VecCoreInstDefn::V1] = 2;
    inst.operands[VecCoreInstDefn::VEC_IDX] = 3;
    prog1.append(inst);
    // receive 4 from core 0
    inst.opcode = VecCoreInstDefn::RECV_SCALAR;
    inst.operands[VecCoreInstDefn::CORE_IDX] = 0;
    inst.operands[VecCoreInstDefn::V1] = 2;
    inst.operands[VecCoreInstDefn::VEC_IDX] = 4;
    prog1.append(inst);

    // send 7 to core 0
    inst.opcode = VecCoreInstDefn::SEND_SCALAR;
    inst.operands[VecCoreInstDefn::CORE_IDX] = 0;
    inst.operands[VecCoreInstDefn::V1] = 2;
    inst.operands[VecCoreInstDefn::VEC_IDX] = 7;
    prog1.append(inst);
    // send 9 to core 0
    inst.opcode = VecCoreInstDefn::SEND_SCALAR;
    inst.operands[VecCoreInstDefn::CORE_IDX] = 0;
    inst.operands[VecCoreInstDefn::V1] = 2;
    inst.operands[VecCoreInstDefn::VEC_IDX] = 9;
    prog1.append(inst);

    // store to memory
    inst.opcode = VecCoreInstDefn::STORE_VEC;
    inst.operands[VecCoreInstDefn::ADDR] = 0;
    inst.operands[VecCoreInstDefn::V1] = 2;
    prog1.append(inst);

    // halt
    inst.opcode = VecCoreInstDefn::HALT;
    prog1.append(inst);

    SaveProgram(prog0.toBinary(), "inst_mem0.txt");
    SaveProgram(prog1.toBinary(), "inst_mem1.txt");

    return 0;
}
