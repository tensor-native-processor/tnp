#ifndef MAT_COMPILER_H_
#define MAT_COMPILER_H_

#include <vector>

#include "compiler_common.h"
#include "mat_program.h"
#include "vec_program.h"

void writeBlockMatToMem(std::vector<std::vector<float>> &mat, int rBlockIdx, int cBlockIdx, bool xflipTranspose, std::ofstream &dm);

void conditionalStoreAndLoad(
    int matBlockReg, int matBlockMemAddr,
    int matMaxRegs, int (&matRegToMemAddr)[MAT_REG_SIZE], MatCoreProgram &matProg);

void loadMatBlocks(int rBlockSize, int cBlockSize, 
int matRegStart, int matMemStart, 
int matMaxRegs, int (&matRegToMemAddr)[MAT_REG_SIZE], MatCoreProgram& matProg);

void sumBlock(
    MatCoreProgram &fromMatProg1, MatCoreProgram &fromMatProg2, VecCoreProgram &vecProg, MatCoreProgram &toMatProg, 
    int fromMatCoreIdx1, int fromMatCoreIdx2, int vecCoreIdx, int toMatCoreIdx,
    int fromMatCoreReg1, int fromMatCoreReg2, int vecReg0, int vecReg1, int vecReg2, int toMatCoreReg);

void singleCoreHelper(
    int matCoreIdx, int vecCoreIdx, 
    MatCoreProgram &matProg, VecCoreProgram &vecProg,
    int matARBlockSize, int matACBlockSize,
    int matBRBlockSize, int matBCBlockSize,
    int matAMemStart, int matBMemStart, int matCMemStart,
    int matARegStart, int matBRegStart, int matCRegStart,
    int matAMaxRegs, int matBMaxRegs, int matCMaxRegs,
    int (&matRegToMemAddr)[MAT_REG_SIZE],
    int tmpReg,
    int vecReg0, int vecReg1, int vecReg2,
    const std::vector<size_t> &regMap
    );

void singleCore(
    std::vector<std::vector<float>> &matA, 
    std::vector<std::vector<float>> &matB,
    std::vector<std::vector<float>> &matC,
    std::vector<std::vector<float>> &matRef);

void multiMultAndAdd(int coresForRows, int coresForCols, 
    std::vector<MatInfo> &subMatInfos, 
    MatCoreProgram (&matProgs)[NUM_MAT_CORES],
    VecCoreProgram (&vecProgs)[NUM_VEC_CORES],
    std::vector<std::tuple<int, int, int>> &addCoreIdxs);

std::tuple<int, int> getCoreAssignment(int matASize, int matA0Size);

void multiCore(
    matrix &matA, 
    matrix &matB,
    matrix &matC,
    matrix &matRef);

#endif