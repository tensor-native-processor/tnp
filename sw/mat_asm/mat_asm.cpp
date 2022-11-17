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
    inst.operands[MatInstruction::Md] = 258;
    prog.append(inst);

    std::cout << prog.toText() << std::endl;

    return 0;
}
