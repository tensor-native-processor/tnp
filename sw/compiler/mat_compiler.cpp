#include "common.h"
#include "mat_program.h"
#include "vec_program.h"

void writeBlockMatToMem(std::vector<std::vector<float>> &mat, int rBlockIdx, int cBlockIdx, bool xflipTranspose, std::ofstream &dm) {
    if (!xflipTranspose) {
        for (int i = rBlockIdx * BLOCK_WIDTH; i < rBlockIdx * BLOCK_WIDTH + BLOCK_WIDTH; i++) {
            for (int j = cBlockIdx * BLOCK_WIDTH; j < cBlockIdx * BLOCK_WIDTH + BLOCK_WIDTH; j++) {
                float elem = 0;
                if (i < mat.size() && j < mat[0].size()) {
                    elem = mat[i][j];
                }
                dm << elem << std::endl;
            }
        }
    } else {
        for (int j = cBlockIdx * BLOCK_WIDTH; j < cBlockIdx * BLOCK_WIDTH + BLOCK_WIDTH; j++) {
            for (int i = rBlockIdx * BLOCK_WIDTH + BLOCK_WIDTH - 1; i >= rBlockIdx * BLOCK_WIDTH; i--) {
                float elem = 0;
                if (i < mat.size() && j < mat[0].size()) {
                    elem = mat[i][j];
                }
                dm << elem << std::endl;
            }
        }
    }
}

void loadMatBlocks(int rBlockSize, int cBlockSize, 
int matRegStart, int matMemStart, 
int matMaxRegs, int (&matRegToMemAddr)[MAT_REG_SIZE], MatCoreProgram& matProg) {
    MatCoreInst matInst;
    bool isMatARegsFull = false;
    for (int rBlockIdx = 0; rBlockIdx < rBlockSize; rBlockIdx++) {
        if (isMatARegsFull) break;
        for (int cBlockIdx = 0; cBlockIdx < cBlockSize; cBlockIdx++) {
            int matABlockOffset = rBlockIdx * cBlockSize + cBlockIdx;
            int matAMemOffset = matABlockOffset * BLOCK_AREA;

            if (matABlockOffset == matMaxRegs) {
                isMatARegsFull = true;
                break;
            }

            // load a mat block 
            matInst.opcode = MatCoreInstDefn::LOAD_MAT;
            int reg = matRegStart + matABlockOffset;
            int addr = matMemStart + matAMemOffset;
            matRegToMemAddr[reg] = addr;
            matInst.operands[MatCoreInstDefn::ADDR] = addr;
            matInst.operands[MatCoreInstDefn::M1] = reg;
            matProg.append(matInst);
        }
    }
}

void conditionalStoreAndLoad(
    int matBlockReg, int matBlockMemAddr,
    int matMaxRegs, int (&matRegToMemAddr)[MAT_REG_SIZE], MatCoreProgram &matProg) {
    MatCoreInst matInst;

    if (matBlockMemAddr != matRegToMemAddr[matBlockReg]) {
        matInst.opcode = MatCoreInstDefn::STORE_MAT;
        matInst.operands[MatCoreInstDefn::ADDR] = matRegToMemAddr[matBlockReg];
        matInst.operands[MatCoreInstDefn::M1] = matBlockReg;
        matProg.append(matInst);

        matInst.opcode = MatCoreInstDefn::LOAD_MAT;
        matInst.operands[MatCoreInstDefn::ADDR] = matBlockMemAddr;
        matInst.operands[MatCoreInstDefn::M1] = matBlockReg;
        matProg.append(matInst);
    }
}


void singleCoreHelper(
    int matCoreIdx, int vecCoreIdx, MatCoreProgram &matProg, VecCoreProgram &vecProg,
    std::vector<std::vector<float>> &matA, 
    std::vector<std::vector<float>> &matB,
    std::vector<std::vector<float>> &matC,
    std::vector<std::vector<float>> &matRef,
    int matARBlockSize, int matACBlockSize,
    int matBRBlockSize, int matBCBlockSize) {

    std::ofstream matDM(getDataMemName(matCoreIdx));
    matDM << std::setprecision(FLOAT_PRECISION) << std::fixed;
    
    for (int rBlockIdx = 0; rBlockIdx < matARBlockSize; rBlockIdx++) {
        for (int cBlockIdx = 0; cBlockIdx < matACBlockSize; cBlockIdx++) {
            writeBlockMatToMem(matA, rBlockIdx, cBlockIdx, true, matDM);
        }
    }
    
    for (int rBlockIdx = 0; rBlockIdx < matBRBlockSize; rBlockIdx++) {
        for (int cBlockIdx = 0; cBlockIdx < matBCBlockSize; cBlockIdx++) {
            writeBlockMatToMem(matB, rBlockIdx, cBlockIdx, false, matDM);
        }
    }

    for (int rBlockIdx = 0; rBlockIdx < matARBlockSize; rBlockIdx++) {
        for (int cBlockIdx = 0; cBlockIdx < matBCBlockSize; cBlockIdx++) {
            writeBlockMatToMem(matC, rBlockIdx, cBlockIdx, false, matDM);
        }
    }
    
    matDM.close(); 

    std::ofstream vecDM(getDataMemName(vecCoreIdx));
    vecDM.close(); 

    int matAMemSize = matARBlockSize * matACBlockSize * BLOCK_AREA;
    int matBMemSize = matBRBlockSize * matBCBlockSize * BLOCK_AREA;
    int matCMemSize = matARBlockSize * matBCBlockSize * BLOCK_AREA;

    int matAMemStart = 0;
    int matBMemStart = matAMemSize;
    int matCMemStart = matBMemStart + matBMemSize;

    // mat core registers
    int matMaxRegs = (MAT_REG_SIZE - 1) / 3;
    int matARegStart = 0;
    int matBRegStart = matMaxRegs;
    int matCRegStart = 2 * matMaxRegs;
    // mat core tmp register
    int tmpReg = MAT_REG_SIZE - 1;

    int matRegToMemAddr[MAT_REG_SIZE];
    for (int i = 0; i < MAT_REG_SIZE; i++) {
        matRegToMemAddr[i] = -1;
    }

    // vec core registers
    int vecReg0 = 0;
    int vecReg1 = 1;
    int vecReg2 = 2;

    MatCoreInst matInst;
    VecCoreInst vecInst;

    loadMatBlocks(matARBlockSize, matACBlockSize,
        matARegStart, matAMemStart, matMaxRegs, matRegToMemAddr, matProg);

    loadMatBlocks(matBRBlockSize, matBCBlockSize,
        matBRegStart, matBMemStart, matMaxRegs, matRegToMemAddr, matProg);

    loadMatBlocks(matARBlockSize, matBCBlockSize,
        matCRegStart, matCMemStart, matMaxRegs, matRegToMemAddr, matProg);

    // matCBlock_rc += matABlock_rk * matBBlock_kc
    for (int rBlockIdx = 0; rBlockIdx < matARBlockSize; rBlockIdx++) {
        for (int kBlockIdx = 0; kBlockIdx < matACBlockSize; kBlockIdx++) {
            int matABlockOffset = rBlockIdx * matACBlockSize + kBlockIdx;
            int matAMemOffset = matABlockOffset * BLOCK_AREA;
            int matABlockReg = matARegStart + matABlockOffset % matMaxRegs;
            int matAMemAddr = matAMemStart + matAMemOffset;
            conditionalStoreAndLoad(matABlockReg, matAMemAddr, matMaxRegs, 
            matRegToMemAddr, matProg);

            for (int cBlockIdx = 0; cBlockIdx < matBCBlockSize; cBlockIdx++) {
                int matBBlockOffset = kBlockIdx * matBCBlockSize + cBlockIdx;
                int matBMemOffset = matBBlockOffset * BLOCK_AREA;
                int matBBlockReg = matBRegStart + matBBlockOffset % matMaxRegs;
                int matBMemAddr = matBMemStart + matBMemOffset; 
                conditionalStoreAndLoad(matBBlockReg, matBMemAddr, matMaxRegs, 
                matRegToMemAddr, matProg);
                
                int matCBlockOffset = rBlockIdx * matBCBlockSize + cBlockIdx; 
                int matCMemOffset = matCBlockOffset * BLOCK_AREA;
                int matCBlockReg = matCRegStart + matCBlockOffset % matMaxRegs;
                int matCMemAddr = matCMemStart + matCMemOffset; 
                conditionalStoreAndLoad(matCBlockReg, matCMemAddr, matMaxRegs, 
                matRegToMemAddr, matProg);

                // set weight of matA into systolic array
                matInst.opcode = MatCoreInstDefn::SET_WEIGHT;
                matInst.operands[MatCoreInstDefn::M1] = matABlockReg;
                matProg.append(matInst);

                // multiply and store result in tmp reg
                matInst.opcode = MatCoreInstDefn::MULTIPLY;
                matInst.operands[MatCoreInstDefn::Md] = tmpReg; 
                matInst.operands[MatCoreInstDefn::M1] = matBBlockReg;
                matProg.append(matInst);

                // send rows to vec core for addition
                for (int i = 0; i < BLOCK_WIDTH; i++) {
                    // send rows of tmp to vec core
                    matInst.opcode = MatCoreInstDefn::SEND_ROW;
                    matInst.operands[MatCoreInstDefn::CORE_IDX] = 4;
                    matInst.operands[MatCoreInstDefn::M1] = tmpReg;
                    matInst.operands[MatCoreInstDefn::ROW_IDX] = i;
                    matProg.append(matInst);

                    vecInst.opcode = VecCoreInstDefn::RECV_VEC;
                    vecInst.operands[VecCoreInstDefn::CORE_IDX] = 0;
                    vecInst.operands[VecCoreInstDefn::V1] = vecReg0;
                    vecProg.append(vecInst);                    

                    // send rows of matC to vec core 
                    matInst.opcode = MatCoreInstDefn::SEND_ROW;
                    matInst.operands[MatCoreInstDefn::CORE_IDX] = 4;
                    matInst.operands[MatCoreInstDefn::M1] = matCBlockReg;
                    matInst.operands[MatCoreInstDefn::ROW_IDX] = i;
                    matProg.append(matInst);

                    vecInst.opcode = VecCoreInstDefn::RECV_VEC;
                    vecInst.operands[VecCoreInstDefn::CORE_IDX] = 0;
                    vecInst.operands[VecCoreInstDefn::V1] = vecReg1;
                    vecProg.append(vecInst);

                    // vec core performs addition
                    vecInst.opcode = VecCoreInstDefn::ADD;
                    vecInst.operands[VecCoreInstDefn::Vd] = vecReg2;
                    vecInst.operands[VecCoreInstDefn::V1] = vecReg0;
                    vecInst.operands[VecCoreInstDefn::V2] = vecReg1;
                    vecProg.append(vecInst);

                    // recv rows from vec core
                    vecInst.opcode = VecCoreInstDefn::SEND_VEC;
                    vecInst.operands[VecCoreInstDefn::CORE_IDX] = 0;
                    vecInst.operands[VecCoreInstDefn::V1] = vecReg2;
                    vecProg.append(vecInst);

                    matInst.opcode = MatCoreInstDefn::RECV_ROW;
                    matInst.operands[MatCoreInstDefn::CORE_IDX] = 4;
                    matInst.operands[MatCoreInstDefn::M1] = matCBlockReg;
                    matInst.operands[MatCoreInstDefn::ROW_IDX] = i;
                    matProg.append(matInst);
                }

                printf("matABlock_%d%d: %7d matBBlock_%d%d: %7d+%7d matCBlock_%d%d: %7d+%7d\n", 
                rBlockIdx, kBlockIdx, matAMemOffset, 
                kBlockIdx, cBlockIdx, matBMemStart, matBMemOffset,
                rBlockIdx, cBlockIdx, matCMemStart, matCMemOffset);
            }
        }
    }

    // write all valid matC regs to mem  
    for (int i = 0; i < matMaxRegs; i++) {
        int matCReg = matCRegStart + i;
        int addr = matRegToMemAddr[matCReg];
        if (addr != -1) {
            matInst.opcode = MatCoreInstDefn::STORE_MAT;
            matInst.operands[MatCoreInstDefn::ADDR] = addr; 
            matInst.operands[MatCoreInstDefn::M1] = matCReg; 
            matProg.append(matInst);
        }
    }


}

void singleCore(
    std::vector<std::vector<float>> &matA, 
    std::vector<std::vector<float>> &matB,
    std::vector<std::vector<float>> &matC,
    std::vector<std::vector<float>> &matRef,
    int matARBlockSize, int matACBlockSize,
    int matBRBlockSize, int matBCBlockSize) {
    
    // output0.txt should be idential to ans0.txt 
    std::ofstream matAns(getAnsName(MAT_CORE_START_IDX));
    matAns << std::setprecision(FLOAT_PRECISION) << std::fixed;

    for (int rBlockIdx = 0; rBlockIdx < matARBlockSize; rBlockIdx++) {
        for (int cBlockIdx = 0; cBlockIdx < matACBlockSize; cBlockIdx++) {
            // writes to ans for reference and easy diffing   
            writeBlockMatToMem(matA, rBlockIdx, cBlockIdx, true, matAns);
        }
    }
    
    for (int rBlockIdx = 0; rBlockIdx < matBRBlockSize; rBlockIdx++) {
        for (int cBlockIdx = 0; cBlockIdx < matBCBlockSize; cBlockIdx++) {
            // writes to ans for reference and easy diffing
            writeBlockMatToMem(matB, rBlockIdx, cBlockIdx, false, matAns);
        }
    }

    for (int rBlockIdx = 0; rBlockIdx < matARBlockSize; rBlockIdx++) {
        for (int cBlockIdx = 0; cBlockIdx < matBCBlockSize; cBlockIdx++) {
            writeBlockMatToMem(matRef, rBlockIdx, cBlockIdx, false, matAns);
        }
    }

    int matAMemSize = matARBlockSize * matACBlockSize * BLOCK_AREA;
    int matBMemSize = matBRBlockSize * matBCBlockSize * BLOCK_AREA;
    int matCMemSize = matARBlockSize * matBCBlockSize * BLOCK_AREA;

    for (int i = 0; i < DATA_MEM_SIZE - (matAMemSize + matBMemSize + matCMemSize); i++) {
        matAns << 0.0 << std::endl;
    }
    matAns.close();

    MatCoreProgram matProg;
    MatCoreInst matInst;
    VecCoreProgram vecProg;
    VecCoreInst vecInst;
    
    singleCoreHelper(MAT_CORE_START_IDX, VEC_CORE_START_IDX,
    matProg, vecProg, matA, matB, matC, matRef,
    matARBlockSize, matACBlockSize, matBRBlockSize, matBCBlockSize);

    // halt mat
    matInst.opcode = MatCoreInstDefn::HALT;
    matProg.append(matInst);
    SaveProgram(matProg.toBinary(), getInstMemName(MAT_CORE_START_IDX));
    
    std::ofstream matTxt(getInstMemTextName(MAT_CORE_START_IDX));
    matTxt << matProg.toText();
    matTxt.close();

    // halt vec    
    vecInst.opcode = VecCoreInstDefn::HALT;
    vecProg.append(vecInst);
    SaveProgram(vecProg.toBinary(), getInstMemName(VEC_CORE_START_IDX));

    std::ofstream vecTxt(getInstMemTextName(VEC_CORE_START_IDX));
    vecTxt << vecProg.toText();
    vecTxt.close();

    // inst, data mem for mat core 1-3
    for (int i = 1; i < 4; i++) {
        MatCoreProgram prog;
        MatCoreInst inst; 
        inst.opcode = MatCoreInstDefn::HALT;
        prog.append(inst);
        SaveProgram(
            prog.toBinary(), getInstMemName(MAT_CORE_START_IDX + i) 
        );
        std::ofstream dm(getDataMemName(MAT_CORE_START_IDX + i));
        dm.close();
    }

    // inst, data mem for vec core 5-7
    for (int i = 1; i < 4; i++) {
        VecCoreProgram prog;
        VecCoreInst inst; 
        inst.opcode = VecCoreInstDefn::HALT;
        prog.append(inst);
        SaveProgram(
            prog.toBinary(), getInstMemName(VEC_CORE_START_IDX + i) 
        );
        std::ofstream dm(getDataMemName(VEC_CORE_START_IDX + i));
        dm.close();
    }
}

void multiCore() {
    //
}

int main(int argc, char *argv[]) {
    std::vector<std::vector<float>> matA;
    std::vector<std::vector<float>> matB;

    /*
    (31, 31) * (31, 17) took 22275 cycles (CPU time 49.670s)
    (64, 128) * (128, 10) took 86115 cycles (CPU time 178.530s)
    (64, 128) * (128, 10) took 88095 cycles if set MAT_REG_SIZE to 64 (CPU time 180.360s)
    */
    // 64 * 128
    for (int i = 0; i < 64; i++) {
        std::vector<float> row;
        for (int j = 0; j < 128; j++) {
           row.push_back(j + 1); 
        }
        matA.push_back(row);
    }
    
    // 128 * 10
    for (int i = 0; i < 128; i++) {
        std::vector<float> row;
        for (int j = 0; j < 10; j++) {
           row.push_back(j + 1); 
        }
        matB.push_back(row);
    }
    
    assert(matA[0].size() == matB.size() && "matA[0].size() != matB.size()");

    std::vector<std::vector<float>> matC(matA.size(), std::vector<float>(matB[0].size(), 0));

    int matARSize = matA.size();
    int matACSize = matA[0].size();
    int matBRSize = matB.size();
    int matBCSize = matB[0].size();

    int matARBlockSize = (matA.size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
    int matACBlockSize = (matA[0].size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
    int matBRBlockSize = (matB.size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
    int matBCBlockSize = (matB[0].size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;

    std::vector<std::vector<float>> matRef = multBruteForce(matA, matB); 

    singleCore(matA, matB, matC, matRef,matARBlockSize, matACBlockSize, matBRBlockSize, matBCBlockSize); 

    return 0;
}