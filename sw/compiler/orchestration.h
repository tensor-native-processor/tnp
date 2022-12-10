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

    // Matrix shape
    struct MatrixShape {
        size_t x, y;

        friend bool operator==(const MatrixShape&, const MatrixShape&);
        friend bool operator!=(const MatrixShape&, const MatrixShape&);
    };

    // Processed matrix result
    struct MatrixResult {
        MatrixShape m_shape;
        size_t m_coreIdx;
        std::vector<std::vector<size_t>> m_dataAddr;
    };
    // Matrix constant
    struct MatrixConstant {
        MatrixShape m_shape;
        std::vector<std::vector<std::vector<float>>> m_data;
    };

    // Matrix handle
    typedef size_t MatrixHandle;

    // Save to files
    void compile();

    // Data operations
    MatrixHandle dataMatrixAllocate(const MatrixShape&);
    void dataMatrixDeallocate(MatrixHandle);
    void dataMatrixLoadConstant(MatrixHandle, const MatrixConstant&);
    MatrixResult dataMatrixStoreResult(MatrixHandle);

    // Arithmetic operations
    MatrixHandle arithmeticMatMult(MatrixHandle, MatrixHandle);
    MatrixHandle arithmeticRelu(MatrixHandle);
    MatrixHandle arithmeticTranspose(MatrixHandle);

private:
    // Orchestrator parameter
    OrchestratorParam m_param;

    // MatCore/VecCore state
    struct MatCoreState {
        MatCoreProgram m_prog;
        std::set<size_t> m_freeRegIdx;
        std::vector<float> m_dataMem;
    };
    struct VecCoreState {
        VecCoreProgram m_prog;
        std::set<size_t> m_freeRegIdx;
        std::vector<float> m_dataMem;
    };

    // Processor state
    struct ProcState {
        std::vector<MatCoreState> matCores;
        std::vector<VecCoreState> vecCores;
    };

    // Programs
    ProcState m_procState;

    // Total matrix handle
    size_t m_matrixHandleCount;

    // Matrix state
    struct MatrixState {
        MatrixState(const MatrixShape&);
        size_t m_coreIdx;
        MatrixShape m_shape;
        std::vector<std::vector<size_t>> m_regIdx;
    };
    std::map<MatrixHandle, MatrixState> m_dataMatrixState;

    // Encode core ID to global IDs
    size_t getMatCoreID(size_t) const;
    size_t getVecCoreID(size_t) const;


    // Arithmetic operation primitives
    MatrixHandle arithmeticReluSingleCore(MatrixHandle);
};

#endif
