#include "mat_program.h"
#include <cstdio>
#include <iostream>

int main() {
    MatProgram prog;

    MatInstruction inst;
    inst.opcode = MatInstruction::SET_WEIGHT;
    inst.operands[MatInstruction::M1] = 258;
    prog.append(inst);

	inst.opcode = MatInstruction::MULTIPLY;
    inst.operands[MatInstruction::Md] = 259;
    prog.append(inst);

	inst.opcode = MatInstruction::HALT;
    prog.append(inst);

    std::cout << prog.toText() << std::endl;

    TNPProgramBinary bin = prog.toBinary();
    MatProgram prog2;
    prog2.fromBinary(bin);
    std::cout << "Next: " << prog2.toText() << std::endl;

    return 0;
}
