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

    // Float precision
    size_t floatPrecision = 10;
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
    MatrixHandle dataMatrixAllocate(const Matrix&);
    void dataMatrixDeallocate(MatrixHandle);
    void dataMatrixBindConstant(MatrixHandle, const float*);

    // Arithmetic operations
    MatrixHandle arithmeticMatMult(MatrixHandle, MatrixHandle);
    MatrixHandle arithmeticRelu(MatrixHandle);

private:
    // Orchestrator parameter
    OrchestratorParam m_param;

    // MatCore/VecCore state
    struct MatCoreState {
        MatCoreProgram prog;
        std::set<size_t> freeRegIdx;
        std::vector<float> dataMem;
    };
    struct VecCoreState {
        VecCoreProgram prog;
        std::set<size_t> freeRegIdx;
        std::vector<float> dataMem;
    };

    // Programs
    std::vector<MatCoreState> m_matCoreStatus;
    std::vector<VecCoreState> m_vecCoreStatus;

    // Total matrix handle
    size_t m_matrixHandleCount;

    // Matrix state
    struct MatrixState {
        MatrixState(size_t, size_t);
        size_t bx, by;
        size_t coreIdx;
        std::vector<std::vector<size_t>> activeRegIdx;
    };
    std::map<MatrixHandle, MatrixState> m_dataMatrixStatus;

    // Encode core ID to global IDs
    size_t getMatCoreID(size_t) const;
    size_t getVecCoreID(size_t) const;
};

#endif
