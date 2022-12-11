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

void conditionalStoreAndLoad(
    int matBlockReg, int matBlockMemAddr,
    int matMaxRegs, int (&matRegToMemAddr)[MAT_REG_SIZE], MatCoreProgram &matProg) {
    MatCoreInst matInst;

    if (matBlockMemAddr != matRegToMemAddr[matBlockReg]) {
        if (matRegToMemAddr[matBlockReg] != -1) {
            matInst.opcode = MatCoreInstDefn::STORE_MAT;
            matInst.operands[MatCoreInstDefn::ADDR] = matRegToMemAddr[matBlockReg];
            matInst.operands[MatCoreInstDefn::M1] = matBlockReg;
            matProg.append(matInst);
        }
        matInst.opcode = MatCoreInstDefn::LOAD_MAT;
        matInst.operands[MatCoreInstDefn::ADDR] = matBlockMemAddr;
        matInst.operands[MatCoreInstDefn::M1] = matBlockReg;
        matProg.append(matInst);
        matRegToMemAddr[matBlockReg] = matBlockMemAddr;
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

            int reg = matRegStart + matABlockOffset;
            int addr = matMemStart + matAMemOffset;
            
            // load a mat block 
            conditionalStoreAndLoad(reg, addr, matMaxRegs, matRegToMemAddr, matProg);
        }
    }
}

void singleCoreHelper(
    int matCoreIdx, int vecCoreIdx, 
    MatCoreProgram &matProg, VecCoreProgram &vecProg,
    std::vector<std::vector<float>> &matA, 
    std::vector<std::vector<float>> &matB,
    std::vector<std::vector<float>> &matC,
    int matARBlockSize, int matACBlockSize,
    int matBRBlockSize, int matBCBlockSize,
    int matAMemStart, int matBMemStart, int matCMemStart,
    int matARegStart, int matBRegStart, int matCRegStart,
    int matAMaxRegs, int matBMaxRegs, int matCMaxRegs,
    int (&matRegToMemAddr)[MAT_REG_SIZE],
    int tmpReg,
    int vecReg0, int vecReg1, int vecReg2
    ) {

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


    MatCoreInst matInst;
    VecCoreInst vecInst;

    loadMatBlocks(matARBlockSize, matACBlockSize,
        matARegStart, matAMemStart, matAMaxRegs, matRegToMemAddr, matProg);

    loadMatBlocks(matBRBlockSize, matBCBlockSize,
        matBRegStart, matBMemStart, matBMaxRegs, matRegToMemAddr, matProg);

    loadMatBlocks(matARBlockSize, matBCBlockSize,
        matCRegStart, matCMemStart, matCMaxRegs, matRegToMemAddr, matProg);

    // matCBlock_rc += matABlock_rk * matBBlock_kc
    for (int rBlockIdx = 0; rBlockIdx < matARBlockSize; rBlockIdx++) {
        for (int kBlockIdx = 0; kBlockIdx < matACBlockSize; kBlockIdx++) {
            int matABlockOffset = rBlockIdx * matACBlockSize + kBlockIdx;
            int matAMemOffset = matABlockOffset * BLOCK_AREA;
            int matABlockReg = matARegStart + matABlockOffset % matAMaxRegs;
            int matAMemAddr = matAMemStart + matAMemOffset;
            conditionalStoreAndLoad(matABlockReg, matAMemAddr, matAMaxRegs, 
            matRegToMemAddr, matProg);

            for (int cBlockIdx = 0; cBlockIdx < matBCBlockSize; cBlockIdx++) {
                int matBBlockOffset = kBlockIdx * matBCBlockSize + cBlockIdx;
                int matBMemOffset = matBBlockOffset * BLOCK_AREA;
                int matBBlockReg = matBRegStart + matBBlockOffset % matBMaxRegs;
                int matBMemAddr = matBMemStart + matBMemOffset; 
                conditionalStoreAndLoad(matBBlockReg, matBMemAddr, matBMaxRegs, 
                matRegToMemAddr, matProg);
                
                int matCBlockOffset = rBlockIdx * matBCBlockSize + cBlockIdx; 
                int matCMemOffset = matCBlockOffset * BLOCK_AREA;
                int matCBlockReg = matCRegStart + matCBlockOffset % matCMaxRegs;
                int matCMemAddr = matCMemStart + matCMemOffset; 
                conditionalStoreAndLoad(matCBlockReg, matCMemAddr, matCMaxRegs, 
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
                    matInst.operands[MatCoreInstDefn::CORE_IDX] = vecCoreIdx;
                    matInst.operands[MatCoreInstDefn::M1] = tmpReg;
                    matInst.operands[MatCoreInstDefn::ROW_IDX] = i;
                    matProg.append(matInst);

                    vecInst.opcode = VecCoreInstDefn::RECV_VEC;
                    vecInst.operands[VecCoreInstDefn::CORE_IDX] = matCoreIdx;
                    vecInst.operands[VecCoreInstDefn::V1] = vecReg0;
                    vecProg.append(vecInst);                    

                    // send rows of matC to vec core 
                    matInst.opcode = MatCoreInstDefn::SEND_ROW;
                    matInst.operands[MatCoreInstDefn::CORE_IDX] = vecCoreIdx;
                    matInst.operands[MatCoreInstDefn::M1] = matCBlockReg;
                    matInst.operands[MatCoreInstDefn::ROW_IDX] = i;
                    matProg.append(matInst);

                    vecInst.opcode = VecCoreInstDefn::RECV_VEC;
                    vecInst.operands[VecCoreInstDefn::CORE_IDX] = matCoreIdx;
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
                    vecInst.operands[VecCoreInstDefn::CORE_IDX] = matCoreIdx;
                    vecInst.operands[VecCoreInstDefn::V1] = vecReg2;
                    vecProg.append(vecInst);

                    matInst.opcode = MatCoreInstDefn::RECV_ROW;
                    matInst.operands[MatCoreInstDefn::CORE_IDX] = vecCoreIdx;
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
    for (int i = 0; i < matCMaxRegs; i++) {
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
    std::vector<std::vector<float>> &matRef) {

    int matARBlockSize = (matA.size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
    int matACBlockSize = (matA[0].size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
    int matBRBlockSize = (matB.size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
    int matBCBlockSize = (matB[0].size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;

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

    int matAMemStart = 0;
    int matBMemStart = matAMemSize;
    int matCMemStart = matBMemStart + matBMemSize;

    // mat core registers, reserve 1 for tmpReg
    int matMaxRegs = (MAT_REG_SIZE - 1) / 3;
    int matAMaxRegs, matBMaxRegs, matCMaxRegs;
    matAMaxRegs = matBMaxRegs = matCMaxRegs = matMaxRegs;
    
    int matARegStart = 0;
    int matBRegStart = matAMaxRegs;
    int matCRegStart = matAMaxRegs + matBMaxRegs;

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

    MatCoreProgram matProg;
    MatCoreInst matInst;
    VecCoreProgram vecProg;
    VecCoreInst vecInst;
    
    singleCoreHelper(
    MAT_CORE_START_IDX, VEC_CORE_START_IDX,
    matProg, vecProg, matA, matB, matC, 
    matARBlockSize, matACBlockSize, matBRBlockSize, matBCBlockSize,
    matAMemStart, matBMemStart, matCMemStart,
    matARegStart, matBRegStart, matCRegStart,
    matAMaxRegs, matBMaxRegs, matCMaxRegs,
    matRegToMemAddr,
    tmpReg,
    vecReg0, vecReg1, vecReg2
    );

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

    // inst, data mem for remaining mat cores
    for (int i = 1; i < NUM_MAT_CORES; i++) {
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

    // inst, data mem for remaining vec cores
    for (int i = 1; i < NUM_VEC_CORES; i++) {
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

std::vector<std::vector<matrix>> getSubMats(matrix &mat, int coresForRows, int coresForCols) {
    std::vector<std::vector<matrix>> subMats;  
    int subMatRows = (mat.size() + coresForRows - 1) / coresForRows;
    int subMatCols = (mat[0].size() + coresForCols - 1) / coresForCols;
     
    for (int rCoreIdx = 0; rCoreIdx < coresForRows; rCoreIdx++) {
        std::vector<matrix> subMatsPerRow;
        int rStart = rCoreIdx * subMatRows;
        int rEnd = std::min((rCoreIdx + 1) * subMatRows, static_cast<int>(mat.size()));
        
        for (int cCoreIdx = 0; cCoreIdx < coresForCols; cCoreIdx++) {
            matrix subMat;
            int cStart = cCoreIdx * subMatCols;
            int cEnd = std::min((cCoreIdx + 1) * subMatCols, static_cast<int>(mat[0].size())); 
            
            for (int i = rStart; i < rEnd; i++) {
                std::vector<float> row(mat[i].begin() + cStart, mat[i].begin() + cEnd);
                subMat.push_back(row);
            }
            subMatsPerRow.push_back(subMat);
        }
        subMats.push_back(subMatsPerRow);
    }

    // check correctness
    int totalSize = 0;
    for (int rCoreIdx = 0; rCoreIdx < coresForRows; rCoreIdx++) {
        for (int cCoreIdx = 0; cCoreIdx < coresForCols; cCoreIdx++) {
            matrix subMat = subMats[rCoreIdx][cCoreIdx];
            for (int i = 0; i < subMat.size(); i++) {
                totalSize += subMat[i].size();
                for (int j = 0; j < subMats[0].size(); j++) {
                    int ii = rCoreIdx * subMatRows + i;
                    int jj = cCoreIdx * subMatCols + j;
                    if (subMat[i][j] != mat[ii][jj]) {
                        printf("rCoreIdx:%d, cCoreIdx:%d, subMat[%d][%d] (%f) != mat[%d][%d] (%f)",
                        rCoreIdx, cCoreIdx, i, j, subMat[i][j], ii, jj, mat[ii][jj]);
                        assert(false);
                    }
                    j += 1;
                    if (j == mat[0].size()) {
                        j = 0;
                        i += 1;
                    }          
                }
            }
        } 
    }
    assert(totalSize == mat.size() * mat[0].size());
    
    return subMats;
} 

void multiCore(
    matrix &matA, 
    matrix &matB,
    matrix &matC,
    matrix &matRef) {

    // divide mats by number of cores
    int largeFactor= 2;
    int smallFactor = 2;
    assert(largeFactor * smallFactor == NUM_MAT_CORES);
    
    // TODO optimization compare row/col of matB vs matA for better division

    // short wide matA
    int coresForRows = smallFactor;
    int coresForCols = largeFactor;

    if (matA.size() > matA[0].size()) {
        // tall narrow matA
        coresForRows = largeFactor;
        coresForCols = smallFactor; 
    }

    std::vector<std::vector<matrix>> subMatsA = getSubMats(matA, coresForRows, coresForCols);     
    std::vector<std::vector<matrix>> subMatsB = getSubMats(matB, coresForRows, coresForCols);     
    std::vector<std::vector<matrix>> subMatsC = getSubMats(matC, coresForRows, coresForCols);     

    MatCoreProgram matProgs[NUM_MAT_CORES];
    VecCoreProgram vecProgs[NUM_VEC_CORES];

    for (int i = 0; i < coresForRows; i++) {
        for (int j = 0; j < coresForCols; j++) {
            /* MULT & ADD 1 (only support 4 cores for now)
                MULT       ADD
            core  A  B
                0: 00 00 --> vec0 -> mat0  
                1: 01 10 /
                2: 10 01 \
                3: 11 11 --> vec3 -> mat3
            */
            matrix subMatA = subMatsA[i][j];
            matrix subMatB; 
            matrix subMatC;
            if ((i == 0 && j == 1) || (i == 1 && j == 0)) {
                subMatB = subMatsB[j][i];
                subMatC = subMatsC[i][i];
            } else {
                subMatB = subMatsB[i][j];
                subMatC = subMatsC[i][j];
            } 

            int subMatARBlockSize = (subMatA.size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
            int subMatACBlockSize = (subMatA[0].size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
            int subMatBRBlockSize = (subMatB.size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
            int subMatBCBlockSize = (subMatB[0].size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;

            int matAMemSize = subMatARBlockSize * subMatACBlockSize * BLOCK_AREA;
            int matBMemSize = subMatBRBlockSize * subMatBCBlockSize * BLOCK_AREA;

            int matAMemStart = 0;
            int matBMemStart = matAMemSize;
            int matCMemStart = matBMemStart + matBMemSize;

            // mat core registers, reserve 1 for tmpReg
            int matMaxRegs = (MAT_REG_SIZE - 1) / 3;
            int matAMaxRegs, matBMaxRegs, matCMaxRegs;
            matAMaxRegs = matBMaxRegs = matCMaxRegs = matMaxRegs;

            int matARegStart = 0;
            int matBRegStart = matAMaxRegs;
            int matCRegStart = matAMaxRegs + matBMaxRegs;
            
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

            // TODO ans.txt for easy diffing

            int matCoreOffset = i * coresForCols + j;
            int vecCoreOffset = i * coresForCols + j;
            
            // MULT do sub matrix multiplication on every core 
            singleCoreHelper(
                MAT_CORE_START_IDX + matCoreOffset, VEC_CORE_START_IDX + vecCoreOffset, 
                matProgs[matCoreOffset], vecProgs[matCoreOffset], 
                subMatA, subMatB, subMatC,  
                subMatARBlockSize, subMatACBlockSize,
                subMatBRBlockSize, subMatBCBlockSize,
                matAMemStart, matBMemStart, matCMemStart,
                matARegStart, matBRegStart, matCRegStart,
                matAMaxRegs, matBMaxRegs, matCMaxRegs,
                matRegToMemAddr,
                tmpReg,
                vecReg0, vecReg1, vecReg2
            );


            // ADD add results using the vector core
            // matInst.opcode = MatCoreInstDefn::SEND_ROW;
            // matInst.operands[MatCoreInstDefn::CORE_IDX] = vecCoreIdx;
            // matInst.operands[MatCoreInstDefn::M1] = tmpReg;
            // matInst.operands[MatCoreInstDefn::ROW_IDX] = i;
            // matProg[matCoreOffset].append(matInst);


            // STORE store result to memory
        }    
    }

    /* EXCHANGE subMatB
        0 (00) <-> 2 (10)
        1 (01) <-> 3 (11)
    */

    // for (int i = 0; i < BLOCK_WIDTH; i++) {
    //     MatCoreInst matInst;
    //     matInst.opcode = MatCoreInstDefn::SEND_ROW;
    //     matInst.operands[MatCoreInstDefn::CORE_IDX] = matCoreIdx1;
    //     matInst.operands[MatCoreInstDefn::M1] = ;
    //     matInst.operands[MatCoreInstDefn::ROW_IDX] = i;

    // }
    // matProgs[i].append()
       

    /* MULT & ADD 2 (only support 4 cores for now)
          MULT       ADD
     core  A  B
        0: 00 01 \   
        1: 01 11 --> vec1 -> mat1
        2: 10 00 --> vec2 -> mat2
        3: 11 10 / 
    */

    /* SEND SUBMATS TO CORE 0

    */

    // inst mem for mat cores
    for (int i = 0; i < NUM_MAT_CORES; i++) {
        MatCoreInst inst; 
        inst.opcode = MatCoreInstDefn::HALT;
        matProgs[i].append(inst);
        SaveProgram(
            matProgs[i].toBinary(), getInstMemName(MAT_CORE_START_IDX + i) 
        );
        std::ofstream txt(getInstMemTextName(MAT_CORE_START_IDX + i));
        txt << matProgs[i].toText();
        txt.close();
    }

    // inst mem for vec cores
    for (int i = 0; i < NUM_VEC_CORES; i++) {
        VecCoreInst inst; 
        inst.opcode = VecCoreInstDefn::HALT;
        vecProgs[i].append(inst);
        SaveProgram(
            vecProgs[i].toBinary(), getInstMemName(VEC_CORE_START_IDX + i) 
        );
        std::ofstream txt(getInstMemTextName(VEC_CORE_START_IDX + i));
        txt << vecProgs[i].toText();
        txt.close();
    }
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
    for (int i = 0; i < 16; i++) {
        std::vector<float> row;
        for (int j = 0; j < 16; j++) {
            int num = j + 1;
            if (i >= 8) num *= 2;
            row.push_back(num); 
        }
        matA.push_back(row);
    }
    
    // 128 * 10
    for (int i = 0; i < 16; i++) {
        std::vector<float> row;
        for (int j = 0; j < 16; j++) {
            int num = j + 1;
            if (i >= 8) num *= 2;
            row.push_back(num); 
        }
        matB.push_back(row);
    }
    
    assert(matA[0].size() == matB.size() && "matA[0].size() != matB.size()");

    // init matC with zeroes
    // TODO is this required? By default is it zero in memory?
    std::vector<std::vector<float>> matC(matA.size(), std::vector<float>(matB[0].size(), 0));
    std::vector<std::vector<float>> matRef = multBruteForce(matA, matB); 

    // singleCore(matA, matB, matC, matRef); 
    multiCore(matA, matB, matC, matRef);
    return 0;
}