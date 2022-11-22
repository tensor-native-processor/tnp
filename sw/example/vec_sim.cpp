#include "vec_program.h"
#include <cstdio>
#include <iostream>

int main() {
    TNPProgramBinary bin = LoadProgram("inst_mem.txt");

    VecCoreProgram prog;
    prog.fromBinary(bin);

    VecCoreSimEngine eng(prog, VecCoreParam{});

    size_t cycle_count = 0;
    while (!eng.isDone()) {
        cycle_count++;
        eng.simulateStep();
    }
    std::cout << "Finished with " << cycle_count << " cycles." << std::endl;

    return 0;
}
