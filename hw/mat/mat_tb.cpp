#include "VMatCoreSimVerilator.h"
#include <iostream>

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    VMatCoreSimVerilator* dut = new VMatCoreSimVerilator;

    dut->clock = 0;
    dut->reset = 1;
    for (int t = 0;t < 10000;t++) {
        dut->clock = !dut->clock;

        if (t == 2) dut->reset = 0;

        dut->eval();

        if (dut->done) {
            printf("t = %d\n", t);
            break;
        }
    }

//    for (int i = 0;i < 2048;i++) std::cout << dut->dump[i] << std::endl;


    delete dut;
    return 0;
}
