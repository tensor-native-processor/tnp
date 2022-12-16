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
            .core_self = getMatCoreID(id),
            .allowNoHalt = true
        }, &swSimEngine);
        m_procState.matCores[id].m_cycleCount = 0;
    }

    // VecCore
    std::vector<VecCoreSimEngine> vecCoreSimEngines;
    for (size_t id = 0;id < m_param.vecCoreCount;id++) {
        vecCoreSimEngines.emplace_back(m_procState.vecCores[id].m_prog,
        VecCoreParam{
            .width = m_param.width,
            .core_self = getVecCoreID(id),
            .allowNoHalt = true
        }, &swSimEngine);
        m_procState.vecCores[id].m_cycleCount = 0;
    }

    // Main event loop
    size_t curCycleCount = 0;
    for (;;) {
        // Termination
        bool terminate = true;
        for (size_t id = 0;id < m_param.matCoreCount;id++) {
            if (!matCoreSimEngines[id].isDone()) {
                terminate = false;
                m_procState.matCores[id].m_cycleCount = curCycleCount;
            }
        }
        for (size_t id = 0;id < m_param.vecCoreCount;id++) {
            if (!vecCoreSimEngines[id].isDone()) {
                terminate = false;
                m_procState.vecCores[id].m_cycleCount = curCycleCount;
            }
        }
        if (terminate) {
            break;
        }

        curCycleCount++;
        for (auto& m : matCoreSimEngines) {
            m.simulateStep();
        }
        for (auto& v : vecCoreSimEngines) {
            v.simulateStep();
        }
        swSimEngine.simulateStep();
    }
}
