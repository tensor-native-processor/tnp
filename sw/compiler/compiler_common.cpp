#include "compiler_common.h"
#include "orchestration.h"

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
    regMap.clear();
    printf("regMap size: %lu\n", regMap.size());
}

MatInfo::MatInfo(const matrix &matAIn, const matrix &matBIn, const matrix &matCIn, 
int matAMemStartIn, int matBMemStartIn, int matCMemStartIn) {
    MatInfo::init(matAIn, matBIn, matCIn);
    matAMemStart = matAMemStartIn;
    matBMemStart = matBMemStartIn;
    matCMemStart = matCMemStartIn;
    regMap.clear();
    printf("regMap size: %lu\n", regMap.size());
}

// From orchestrator
MatInfo::MatInfo(
    // These registers are just used to calculate the matrix shapes
    const std::vector<std::vector<size_t>> &m1Reg,
    const std::vector<std::vector<size_t>> &m2Reg,
    const std::vector<std::vector<size_t>> &m3Reg,
    // These registers are the actual ones used
    const std::vector<size_t> &freeRegIdx,
    int dataMemStart
) {
    matARBlockSize = m1Reg.size(); 
    matACBlockSize = m1Reg[0].size();
    matBRBlockSize = m2Reg.size(); 
    matBCBlockSize = m2Reg[0].size();

    // memory    
    matAMemSize = matARBlockSize * matACBlockSize * BLOCK_AREA;
    matBMemSize = matBRBlockSize * matBCBlockSize * BLOCK_AREA;
    matCMemSize = matARBlockSize * matBCBlockSize * BLOCK_AREA;   

    matAMemStart = dataMemStart;
    matBMemStart = matAMemStart + matAMemSize;
    matCMemStart = matBMemStart + matBMemSize;

    // mat core registers, reserve 1 for tmpReg
    int matMaxRegs = (freeRegIdx.size() - 1) / 3;
    matAMaxRegs = matBMaxRegs = matCMaxRegs = matMaxRegs;
    
    matARegStart = 0;
    matBRegStart = matARegStart + matAMaxRegs;
    matCRegStart = matBRegStart + matBMaxRegs;

    // mat core tmp register
    tmpReg = MAT_REG_SIZE - 1;

    for (int i = 0; i < MAT_REG_SIZE; i++) {
        matRegToMemAddr[i] = -1;
    }

    // must init map since registers already loaded the matrices 
    for (int i = 0; i < matAMaxRegs; i++) {
        matRegToMemAddr[matARegStart + i] = matAMemStart + i * BLOCK_AREA;
    }
    for (int i = 0; i < matBMaxRegs; i++) {
        matRegToMemAddr[matBRegStart + i] = matBMemStart + i * BLOCK_AREA;
    }
    for (int i = 0; i < matCMaxRegs; i++) {
        matRegToMemAddr[matCRegStart + i] = matCMemStart + i * BLOCK_AREA;
    }
    
    // vec core registers
    vecReg0 = 0;
    vecReg1 = 1;
    vecReg2 = 2;

    regMap = freeRegIdx;
    printf("regMap size: %lu\n", regMap.size());
}
