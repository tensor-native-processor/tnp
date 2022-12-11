#include "compiler_common.h"

void MatInfo::init(const matrix &matAIn, const matrix &matBIn, const matrix &matCIn){
    matA = matAIn;
    matB = matBIn;
    matC = matCIn; 

    matARBlockSize = (matA.size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
    matACBlockSize = (matA[0].size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
    matBRBlockSize = (matB.size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
    matBCBlockSize = (matB[0].size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;

    // memory    
    matAMemSize = matARBlockSize * matACBlockSize * BLOCK_AREA;
    matBMemSize = matBRBlockSize * matBCBlockSize * BLOCK_AREA;
    matCMemSize = matARBlockSize * matBCBlockSize * BLOCK_AREA;   

    matAMemStart = 0;
    matBMemStart = matAMemSize;
    matCMemStart = matBMemStart + matBMemSize;

    // mat core registers, reserve 1 for tmpReg
    int matMaxRegs = (MAT_REG_SIZE - 1) / 3;
    matAMaxRegs = matBMaxRegs = matCMaxRegs = matMaxRegs;
    
    matARegStart = 0;
    matBRegStart = matAMaxRegs;
    matCRegStart = matAMaxRegs + matBMaxRegs;

    // mat core tmp register
    tmpReg = MAT_REG_SIZE - 1;

    for (int i = 0; i < MAT_REG_SIZE; i++) {
        matRegToMemAddr[i] = -1;
    }

    // vec core registers
    vecReg0 = 0;
    vecReg1 = 1;
    vecReg2 = 2;
}

MatInfo::MatInfo(const matrix &matAIn, const matrix &matBIn, const matrix &matCIn){
    MatInfo::init(matAIn, matBIn, matCIn);
}

MatInfo::MatInfo(const matrix &matAIn, const matrix &matBIn, const matrix &matCIn, 
int matAMemStartIn, int matBMemStartIn, int matCMemStartIn) {
    MatInfo::init(matAIn, matBIn, matCIn);
    matAMemStart = matAMemStartIn;
    matBMemStart = matBMemStartIn;
    matCMemStart = matCMemStartIn;
}
