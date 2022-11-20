#include "mat_program.h"
#include <cstdio>
#include <iostream>

int main() {
    TNPProgramBinary bin = LoadProgram("inst_mem.txt");

    MatCoreProgram prog;
    prog.fromBinary(bin);

    MatCoreSimEngine eng(prog, MatCoreParam{});

    size_t cycle_count = 0;
    while (!eng.isDone()) {
        cycle_count++;
        eng.simulateStep();
    }
    std::cout << "Finished with " << cycle_count << " cycles." << std::endl;

    return 0;
}
