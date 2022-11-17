#include "mat_program.h"
#include <cstdio>
#include <iostream>

int main() {
    MatCoreProgram prog;

    MatCoreInst inst;
    inst.opcode = MatCoreInstDefn::SET_WEIGHT;
    inst.operands[MatCoreInstDefn::M1] = 258;
    prog.append(inst);

	inst.opcode = MatCoreInstDefn::MULTIPLY;
    inst.operands[MatCoreInstDefn::Md] = 259;
    prog.append(inst);

	inst.opcode = MatCoreInstDefn::HALT;
    prog.append(inst);

    std::cout << prog.toText() << std::endl;

    TNPProgramBinary bin = prog.toBinary();
    MatCoreProgram prog2;
    prog2.fromBinary(bin);
    std::cout << "Next: " << prog2.toText() << std::endl;

    return 0;
}
