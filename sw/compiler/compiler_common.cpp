#include "error.h"
#include "logging.h"
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
    regMap.clear();
    LogInfo("regMap size: " + std::to_string(regMap.size()));
}

MatInfo::MatInfo(const matrix &matAIn, const matrix &matBIn, const matrix &matCIn, 
int matAMemStartIn, int matBMemStartIn, int matCMemStartIn) {
    MatInfo::init(matAIn, matBIn, matCIn);
    matAMemStart = matAMemStartIn;
    matBMemStart = matBMemStartIn;
    matCMemStart = matCMemStartIn;
    regMap.clear();
    LogInfo("regMap size: " + std::to_string(regMap.size()));
}

void errorIfNotEnoughRegisters(int matMaxRegs, int matRegsRequired, std::string matType) {
    if (matMaxRegs < matRegsRequired) {
        FatalError("Error: max registers " + std::to_string(matMaxRegs) + 
        "< required register " + std::to_string(matRegsRequired) + "in" + matType);
    }
}

// From orchestrator
MatInfo::MatInfo(
    const int coreIdx,
    const std::vector<std::vector<size_t>> &m1Reg,
    const std::vector<std::vector<size_t>> &m2Reg,
    const std::vector<std::vector<size_t>> &m3Reg,
    const std::vector<size_t> &freeRegIdx,
    int dataMemStart
) {
    LogInfo("Creating MatInfo for coreIdx " + std::to_string(coreIdx)); 
    std::cout << "m3Reg ";
    for (auto &r : m3Reg) {
        for (size_t reg: r) {
            std::cout << reg;
        }
    }
    std::cout << std::endl;
    matAReg = m1Reg;
    matBReg = m2Reg;
    matCReg = m3Reg;

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

    regMap = freeRegIdx;

    // mat core registers, reserve 1 for tmpReg
    const int maxRegSize = regMap.size();
    int matMaxRegs = (maxRegSize - 1) / 3;
    matAMaxRegs = matBMaxRegs = matCMaxRegs = matMaxRegs;
    
    errorIfNotEnoughRegisters(matAMaxRegs, matAMemSize / BLOCK_AREA, "matA");
    errorIfNotEnoughRegisters(matBMaxRegs, matBMemSize / BLOCK_AREA, "matB");
    errorIfNotEnoughRegisters(matCMaxRegs, matCMemSize / BLOCK_AREA, "matC");
    
    matARegStart = 0;
    matBRegStart = matARegStart + matAMaxRegs;
    matCRegStart = matBRegStart + matBMaxRegs;
    LogInfo("matARegStart " + std::to_string(matARegStart) + 
        " matBRegStart " + std::to_string(matBRegStart) +
        " matCRegStart " + std::to_string(matCRegStart));

    // mat core tmp register
    tmpReg = maxRegSize - 1;

    for (int i = 0; i < maxRegSize; i++) {
        matRegToMemAddr[i] = -1;
    }

    // must init map since registers already loaded the matrices 
    for (int i = 0; i < matAMaxRegs; i++) {
        matRegToMemAddr[regMap[matARegStart + i]] = matAMemStart + i * BLOCK_AREA;
    }
    for (int i = 0; i < matBMaxRegs; i++) {
        matRegToMemAddr[regMap[matBRegStart + i]] = matBMemStart + i * BLOCK_AREA;
    }
    for (int i = 0; i < matCMaxRegs; i++) {
        matRegToMemAddr[regMap[matCRegStart + i]] = matCMemStart + i * BLOCK_AREA;
    }
    
    // vec core registers
    vecReg0 = 0;
    vecReg1 = 1;
    vecReg2 = 2;


    LogInfo("regMap size: " + std::to_string(regMap.size()));
}
