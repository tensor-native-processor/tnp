#include "mat_program.h"
#include <cstdio>
#include <iostream>

int main() {
    MatInstruction inst;
    inst.op = MatInstruction::SET_WEIGHT;
    inst.M1 = 258;

    MatProgram prog;
    prog.append(inst);
	inst.Md = 89;
	inst.op = MatInstruction::MULTIPLY;
    prog.append(inst);
    std::cout << prog.toText() << std::endl;

    return 0;
}
