#include "mat_program.h"
#include "mat_sim.h"
#include "vec_program.h"
#include "vec_sim.h"
#include <cstdio>
#include <iostream>
#include <string>

int main() {
    // Switch
    SwitchSimEngine sw_sim(SwitchParam{
        .core_size = 8
    });

    // MatCore
    std::vector<MatCoreSimEngine> mat_sim;
    for (size_t i = 0;i < 4;i++) {
        MatCoreProgram prog;
        prog.fromBinary(LoadProgram("inst_mem" + std::to_string(i) + ".txt"));
        mat_sim.emplace_back(prog, MatCoreParam{
            .core_self = i,
            .memoryPenalty = 100
        }, &sw_sim);
    }

    // VecCore
    std::vector<VecCoreSimEngine> vec_sim;
    for (size_t i = 4;i < 8;i++) {
        VecCoreProgram prog;
        prog.fromBinary(LoadProgram("inst_mem" + std::to_string(i) + ".txt"));
        vec_sim.emplace_back(prog, VecCoreParam{
            .core_self = i,
            .memoryPenalty = 100
        }, &sw_sim);
    }

    // Main event loop
    size_t cycle_count = 0;
    for (;;) {
        // Termination
        bool terminate = true;
        for (const auto& m : mat_sim) {
            if (!m.isDone()) {
                terminate = false;
            }
        }
        for (const auto& v : vec_sim) {
            if (!v.isDone()) {
                terminate = false;
            }
        }
        if (terminate) {
            break;
        }

        cycle_count++;
        for (auto& m : mat_sim) {
            m.simulateStep();
        }
        for (auto& v : vec_sim) {
            v.simulateStep();
        }
        sw_sim.simulateStep();
    }
    std::cout << "Finished with " << cycle_count << " cycles." << std::endl;

    for (size_t i = 0;i < mat_sim.size();i++) {
        std::cout << "MatCore " << i << ":" << std::endl;
        mat_sim[i].printStat();
        std::cout << "================" << std::endl;
    }
    for (size_t i = 0;i < vec_sim.size();i++) {
        std::cout << "VecCore " << i << ":" << std::endl;
        vec_sim[i].printStat();
        std::cout << "================" << std::endl;
    }

    return 0;
}
