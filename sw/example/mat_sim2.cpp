#include "mat_program.h"
#include "mat_sim.h"
#include <cstdio>
#include <iostream>

int main() {
    MatCoreProgram prog0, prog1;
    prog0.fromBinary(LoadProgram("inst_mem0.txt"));
    prog1.fromBinary(LoadProgram("inst_mem1.txt"));

    SwitchSimEngine sw(SwitchParam{
        .core_size = 2
    });
    MatCoreSimEngine mat0(prog0, MatCoreParam{
        .core_self = 0,
    }, &sw);
    MatCoreSimEngine mat1(prog1, MatCoreParam{
        .core_self = 1
    }, &sw);

    size_t cycle_count = 0;
    while (!mat0.isDone() || !mat1.isDone()) {
        cycle_count++;
        mat0.simulateStep();
        mat1.simulateStep();
        sw.simulateStep();
    }
    std::cout << "Finished with " << cycle_count << " cycles." << std::endl;

    return 0;
}
