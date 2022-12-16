#include "orchestration.h"

#include "switch.h"
#include "mat_program.h"
#include "mat_sim.h"
#include "vec_program.h"
#include "vec_sim.h"

#include <vector>
#include <iostream>

// Update m_cycleCount for core states
void Orchestrator::simulateCycleCount() {
    // Switch
    SwitchSimEngine swSimEngine(SwitchParam{
        .width = m_param.width,
        .core_size = m_param.matCoreCount + m_param.vecCoreCount
    });

    // MatCore
    std::vector<MatCoreSimEngine> matCoreSimEngines;
    for (size_t id = 0;id < m_param.matCoreCount;id++) {
        matCoreSimEngines.emplace_back(m_procState.matCores[id].m_prog,
        MatCoreParam{
            .width = m_param.width,
            .core_self = getMatCoreID(id)
        }, &swSimEngine);
    }

    // VecCore
    std::vector<VecCoreSimEngine> vecCoreSimEngines;
    for (size_t id = 0;id < m_param.vecCoreCount;id++) {
        vecCoreSimEngines.emplace_back(m_procState.vecCores[id].m_prog,
        VecCoreParam{
            .width = m_param.width,
            .core_self = getVecCoreID(id)
        }, &swSimEngine);
    }

    // Main event loop
    size_t cycle_count = 0;
    for (;;) {
        // Termination
        bool terminate = true;
        for (const auto& m : matCoreSimEngines) {
            if (!m.isDone()) {
                terminate = false;
            }
        }
        for (const auto& v : vecCoreSimEngines) {
            if (!v.isDone()) {
                terminate = false;
            }
        }
        if (terminate) {
            break;
        }

        cycle_count++;
        for (auto& m : matCoreSimEngines) {
            m.simulateStep();
        }
        for (auto& v : vecCoreSimEngines) {
            v.simulateStep();
        }
        swSimEngine.simulateStep();
    }
    std::cout << "Finished with " << cycle_count << " cycles." << std::endl;
}
