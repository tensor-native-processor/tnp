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

    // Matrix
    struct Matrix {
        size_t x, y;
    };

    // Matrix handle
    typedef size_t MatrixHandle;

    // Save to files
    void compile();

    // Data operations
    MatrixHandle dataAllocate(const Matrix&);
    void dataDeallocate(MatrixHandle);
    void dataBindConstant(MatrixHandle, const float*);

    // Arithmetic operations
    MatrixHandle arithmeticMatMult(MatrixHandle, MatrixHandle);
    MatrixHandle arithmeticRelu(MatrixHandle);

private:
    // Orchestrator parameter
    OrchestratorParam m_param;

    // Programs
    std::vector<MatCoreProgram> m_matProgs;
    std::vector<VecCoreProgram> m_vecProgs;

    // Encode core ID to global IDs
    size_t getMatCoreID(size_t) const;
    size_t getVecCoreID(size_t) const;
};

#endif
