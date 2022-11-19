#include "mat_program.h"
#include <cstdio>
#include <iostream>

int main() {
    MatCoreProgram prog;
    MatCoreInst inst;

    // halt
    inst.opcode = MatCoreInstDefn::HALT;
    prog.append(inst);

    TNPProgramBinary bin = prog.toBinary();
    SaveProgram(bin, "inst_mem.txt");

    return 0;
}
