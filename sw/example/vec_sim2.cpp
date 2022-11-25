#include "vec_program.h"
#include "vec_sim.h"
#include <cstdio>
#include <iostream>

int main() {
    VecCoreProgram prog0, prog1;
    prog0.fromBinary(LoadProgram("inst_mem0.txt"));
    prog1.fromBinary(LoadProgram("inst_mem1.txt"));

    SwitchSimEngine sw(SwitchParam{
        .core_size = 2
    });
    VecCoreSimEngine vec0(prog0, VecCoreParam{
        .core_self = 0
    }, &sw);
    VecCoreSimEngine vec1(prog1, VecCoreParam{
        .core_self = 1
    }, &sw);

    size_t cycle_count = 0;
    while (!vec0.isDone() || !vec1.isDone()) {
        cycle_count++;
        vec0.simulateStep();
        vec1.simulateStep();
        sw.simulateStep();
    }
    std::cout << "Finished with " << cycle_count << " cycles." << std::endl;

    return 0;
}
