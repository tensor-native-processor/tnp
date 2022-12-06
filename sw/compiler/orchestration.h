#ifndef __ORCHESTRATION_H__
#define __ORCHESTRATION_H__

#include <vector>
#include "mat_program.h"
#include "vec_program.h"

struct OrchestratorParam {
    size_t width;

    // MatCore
    size_t matCacheSize;
    size_t matCoreCount;

    // VecCore
    size_t vecCacheSize;
    size_t vecCoreCount;
};

class Orchestrator {
public:
    Orchestrator(const OrchestratorParam&);

    void save();

private:
    OrchestratorParam m_param;

    std::vector<MatCoreProgram> m_matProgs;
    std::vector<VecCoreProgram> m_vecProgs;

    size_t getMatCoreID(size_t) const;
    size_t getVecCoreID(size_t) const;
};

#endif
