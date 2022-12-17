#include "logging.h"
#include "mat_program.h"
#include "vec_program.h"
#include "compiler_common.h"
#include "mat_compiler.h"

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

// load a value from matBlockMemAddr to matBlockReg
// if matBlockReg previously stores a value that comes from a different address
// write it back to memory first.
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

void sumBlock(
    MatCoreProgram &fromMatProg1, MatCoreProgram &fromMatProg2, VecCoreProgram &vecProg, MatCoreProgram &toMatProg, 
    int fromMatCoreIdx1, int fromMatCoreIdx2, int vecCoreIdx, int toMatCoreIdx,
    int fromMatCoreReg1, int fromMatCoreReg2, int vecReg0, int vecReg1, int vecReg2, int toMatCoreReg) {
    MatCoreInst matInst;
    VecCoreInst vecInst;
    // send rows to vec core for addition
    for (int i = 0; i < BLOCK_WIDTH; i++) {
        // send rows of mat1 to vec core
        matInst.opcode = MatCoreInstDefn::SEND_ROW;
        matInst.operands[MatCoreInstDefn::CORE_IDX] = vecCoreIdx;
        matInst.operands[MatCoreInstDefn::M1] = fromMatCoreReg1;
        matInst.operands[MatCoreInstDefn::ROW_IDX] = i;
        fromMatProg1.append(matInst);

        vecInst.opcode = VecCoreInstDefn::RECV_VEC;
        vecInst.operands[VecCoreInstDefn::CORE_IDX] = fromMatCoreIdx1;
        vecInst.operands[VecCoreInstDefn::V1] = vecReg0;
        vecProg.append(vecInst);                    

        // send rows of mat2 to vec core 
        matInst.opcode = MatCoreInstDefn::SEND_ROW;
        matInst.operands[MatCoreInstDefn::CORE_IDX] = vecCoreIdx;
        matInst.operands[MatCoreInstDefn::M1] = fromMatCoreReg2;
        matInst.operands[MatCoreInstDefn::ROW_IDX] = i;
        fromMatProg2.append(matInst);

        vecInst.opcode = VecCoreInstDefn::RECV_VEC;
        vecInst.operands[VecCoreInstDefn::CORE_IDX] = fromMatCoreIdx2;
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
        vecInst.operands[VecCoreInstDefn::CORE_IDX] = toMatCoreIdx;
        vecInst.operands[VecCoreInstDefn::V1] = vecReg2;
        vecProg.append(vecInst);

        matInst.opcode = MatCoreInstDefn::RECV_ROW;
        matInst.operands[MatCoreInstDefn::CORE_IDX] = vecCoreIdx;
        matInst.operands[MatCoreInstDefn::M1] = toMatCoreReg;
        matInst.operands[MatCoreInstDefn::ROW_IDX] = i;
        toMatProg.append(matInst);
    }
}

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
    ) {

    MatCoreInst matInst;
    VecCoreInst vecInst;

    // REGMAP no need to load if orchestrator
    if (regMap.size() == 0) {
        loadMatBlocks(matARBlockSize, matACBlockSize,
            matARegStart, matAMemStart, matAMaxRegs, matRegToMemAddr, matProg);

        loadMatBlocks(matBRBlockSize, matBCBlockSize,
            matBRegStart, matBMemStart, matBMaxRegs, matRegToMemAddr, matProg);

        loadMatBlocks(matARBlockSize, matBCBlockSize,
            matCRegStart, matCMemStart, matCMaxRegs, matRegToMemAddr, matProg);
    }
    // matCBlock_rc += matABlock_rk * matBBlock_kc
    for (int rBlockIdx = 0; rBlockIdx < matARBlockSize; rBlockIdx++) {
        for (int kBlockIdx = 0; kBlockIdx < matACBlockSize; kBlockIdx++) {
            int matABlockOffset = rBlockIdx * matACBlockSize + kBlockIdx;
            int matAMemOffset = matABlockOffset * BLOCK_AREA;

            int matABlockReg = matARegStart + matABlockOffset % matAMaxRegs;
            // REGMAP
            if (regMap.size() > 0) {
                matABlockReg = regMap[matABlockReg];
            } else {
                int matAMemAddr = matAMemStart + matAMemOffset;
                conditionalStoreAndLoad(matABlockReg, matAMemAddr, matAMaxRegs, 
                matRegToMemAddr, matProg);
            }

            for (int cBlockIdx = 0; cBlockIdx < matBCBlockSize; cBlockIdx++) {
                int matBBlockOffset = kBlockIdx * matBCBlockSize + cBlockIdx;
                int matBMemOffset = matBBlockOffset * BLOCK_AREA;

                int matBBlockReg = matBRegStart + matBBlockOffset % matBMaxRegs;
                // REGMAP
                if (regMap.size() > 0) {
                     matBBlockReg = regMap[matBBlockReg];
                } else {
                    int matBMemAddr = matBMemStart + matBMemOffset; 
                    conditionalStoreAndLoad(matBBlockReg, matBMemAddr, matBMaxRegs, 
                    matRegToMemAddr, matProg);
                }
                
                int matCBlockOffset = rBlockIdx * matBCBlockSize + cBlockIdx; 
                int matCMemOffset = matCBlockOffset * BLOCK_AREA;
                
                int matCBlockReg = matCRegStart + matCBlockOffset % matCMaxRegs;
                // REGMAP
                if (regMap.size() > 0) {
                    matCBlockReg = regMap[matCBlockReg];
                    matProg.append({MatCoreInstDefn::CLEAR, {
                        {MatCoreInstDefn::M1, matCBlockReg}
                    }}); 
                } else {
                    int matCMemAddr = matCMemStart + matCMemOffset; 
                    conditionalStoreAndLoad(matCBlockReg, matCMemAddr, matCMaxRegs, 
                    matRegToMemAddr, matProg);
                }

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
                sumBlock(
                    matProg, matProg, vecProg, matProg,
                    matCoreIdx, matCoreIdx, vecCoreIdx, matCoreIdx,
                    tmpReg, matCBlockReg, vecReg0, vecReg1, vecReg2, matCBlockReg
                );

                std::stringstream ss;
                ss << "matABlock_" << rBlockIdx << kBlockIdx << std::setw(7) << matAMemOffset;
                ss << "matBBlock_" << kBlockIdx << cBlockIdx << std::setw(7) << matBMemStart;
                ss << "+" << std::setw(7) << matBMemOffset;
                ss << "matCBlock_" << rBlockIdx << cBlockIdx << std::setw(7) << matCMemStart;
                ss << "+" << std::setw(7) << matCMemOffset;
                LogInfo(ss.str());
            }
        }
    }

    // write all valid matC regs to mem
    // not needed if called from ochestrator
    if (regMap.size() == 0) {
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
}

void singleCore(
    std::vector<std::vector<float>> &matA, 
    std::vector<std::vector<float>> &matB,
    std::vector<std::vector<float>> &matC,
    std::vector<std::vector<float>> &matRef) {

    MatInfo mi(matA, matB, matC);

    // output0.txt should be idential to ans0.txt 
    std::ofstream matAns(getAnsName(MAT_CORE_START_IDX));
    matAns << std::setprecision(FLOAT_PRECISION) << std::fixed;

    for (int rBlockIdx = 0; rBlockIdx < mi.matARBlockSize; rBlockIdx++) {
        for (int cBlockIdx = 0; cBlockIdx < mi.matACBlockSize; cBlockIdx++) {
            // writes to ans for reference and easy diffing   
            writeBlockMatToMem(matA, rBlockIdx, cBlockIdx, true, matAns);
        }
    }
    
    for (int rBlockIdx = 0; rBlockIdx < mi.matBRBlockSize; rBlockIdx++) {
        for (int cBlockIdx = 0; cBlockIdx < mi.matBCBlockSize; cBlockIdx++) {
            // writes to ans for reference and easy diffing
            writeBlockMatToMem(matB, rBlockIdx, cBlockIdx, false, matAns);
        }
    }

    for (int rBlockIdx = 0; rBlockIdx < mi.matARBlockSize; rBlockIdx++) {
        for (int cBlockIdx = 0; cBlockIdx < mi.matBCBlockSize; cBlockIdx++) {
            writeBlockMatToMem(matRef, rBlockIdx, cBlockIdx, false, matAns);
        }
    }

    for (int i = 0; i < DATA_MEM_SIZE - (mi.matAMemSize + mi.matBMemSize + mi.matCMemSize); i++) {
        matAns << 0.0 << std::endl;
    }
    matAns.close();

    std::ofstream matDM(getDataMemName(0));
    matDM << std::setprecision(FLOAT_PRECISION) << std::fixed;
    
    for (int rBlockIdx = 0; rBlockIdx < mi.matARBlockSize; rBlockIdx++) {
        for (int cBlockIdx = 0; cBlockIdx < mi.matACBlockSize; cBlockIdx++) {
            writeBlockMatToMem(matA, rBlockIdx, cBlockIdx, true, matDM);
        }
    }
    
    for (int rBlockIdx = 0; rBlockIdx < mi.matBRBlockSize; rBlockIdx++) {
        for (int cBlockIdx = 0; cBlockIdx < mi.matBCBlockSize; cBlockIdx++) {
            writeBlockMatToMem(matB, rBlockIdx, cBlockIdx, false, matDM);
        }
    }

    for (int rBlockIdx = 0; rBlockIdx < mi.matARBlockSize; rBlockIdx++) {
        for (int cBlockIdx = 0; cBlockIdx < mi.matBCBlockSize; cBlockIdx++) {
            writeBlockMatToMem(matC, rBlockIdx, cBlockIdx, false, matDM);
        }
    }
    matDM.close(); 

    MatCoreProgram matProg;
    MatCoreInst matInst;
    VecCoreProgram vecProg;
    VecCoreInst vecInst;
    
    singleCoreHelper(
        MAT_CORE_START_IDX, VEC_CORE_START_IDX,
        matProg, vecProg, 
        mi.matARBlockSize, mi.matACBlockSize, mi.matBRBlockSize, mi.matBCBlockSize,
        mi.matAMemStart, mi.matBMemStart, mi.matCMemStart,
        mi.matARegStart, mi.matBRegStart, mi.matCRegStart,
        mi.matAMaxRegs, mi.matBMaxRegs, mi.matCMaxRegs,
        mi.matRegToMemAddr,
        mi.tmpReg,
        mi.vecReg0, mi.vecReg1, mi.vecReg2,
        mi.regMap
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

    std::ofstream dm(getDataMemName(VEC_CORE_START_IDX));
    dm.close();

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

void multiMultAndAdd(int coresForRows, int coresForCols, 
    std::vector<MatInfo> &subMatInfos, 
    MatCoreProgram (&matProgs)[NUM_MAT_CORES],
    VecCoreProgram (&vecProgs)[NUM_VEC_CORES],
    std::vector<std::tuple<int, int, int>> &addCoreIdxs) {
    for (int i = 0; i < coresForRows; i++) {
        for (int j = 0; j < coresForCols; j++) {
            int matCoreOffset = i * coresForCols + j;
            int vecCoreOffset = i * coresForCols + j;
            int matCoreIdx = MAT_CORE_START_IDX + matCoreOffset;
            int vecCoreIdx = VEC_CORE_START_IDX + vecCoreOffset;

            MatInfo &mi = subMatInfos[matCoreOffset];

            // MULT do sub matrix multiplication on every core 
            singleCoreHelper(
                matCoreIdx, vecCoreIdx, 
                matProgs[matCoreOffset], vecProgs[matCoreOffset],
                mi.matARBlockSize, mi.matACBlockSize, mi.matBRBlockSize, mi.matBCBlockSize,
                mi.matAMemStart, mi.matBMemStart, mi.matCMemStart,
                mi.matARegStart, mi.matBRegStart, mi.matCRegStart,
                mi.matAMaxRegs, mi.matBMaxRegs, mi.matCMaxRegs,
                mi.matRegToMemAddr,
                mi.tmpReg,
                mi.vecReg0, mi.vecReg1, mi.vecReg2,
                mi.regMap
            );
        }    
    }

    for (int i = 0; i < addCoreIdxs.size(); i++) {
        auto [fromCoreIdx1, fromCoreIdx2, toCoreIdx] = addCoreIdxs[i];
        MatInfo &fromMi1 = subMatInfos[fromCoreIdx1];
        MatInfo &fromMi2 = subMatInfos[fromCoreIdx2];
        MatInfo &toMi = subMatInfos[toCoreIdx];

        MatCoreProgram &fromMp1 = matProgs[fromCoreIdx1];
        MatCoreProgram &fromMp2 = matProgs[fromCoreIdx2];
        MatCoreProgram &toMp = matProgs[toCoreIdx];
        // ADD uses the vec core of mat core `toCoreIdx` for addition
        VecCoreProgram &vp = vecProgs[toCoreIdx];

        for (int rBlockIdx = 0; rBlockIdx < fromMi1.matARBlockSize; rBlockIdx++) {
            for (int cBlockIdx = 0; cBlockIdx < fromMi1.matBCBlockSize; cBlockIdx++) {
                int matCBlockOffset = rBlockIdx * fromMi1.matBCBlockSize + cBlockIdx; 
                int matCMemOffset = matCBlockOffset * BLOCK_AREA;
                
                int matCBlockReg1 = fromMi1.matCRegStart + matCBlockOffset % fromMi1.matCMaxRegs;                
                int matCBlockReg2 = fromMi2.matCRegStart + matCBlockOffset % fromMi2.matCMaxRegs;                
                int toMatCBlockReg = toMi.matCRegStart + matCBlockOffset % toMi.matCMaxRegs;               
                
                // REGMAP
                if (fromMi1.regMap.size() == 0) {
                    int matCMemAddr1 = fromMi1.matCMemStart + matCMemOffset; 
                    conditionalStoreAndLoad(matCBlockReg1, matCMemAddr1, 
                    fromMi1.matCMaxRegs, fromMi1.matRegToMemAddr, fromMp1);
                } else {
                    matCBlockReg1 = fromMi1.regMap[matCBlockReg1];
                }
                
                // REGMAP
                if (fromMi1.regMap.size() == 0) { 
                    int matCMemAddr2 = fromMi2.matCMemStart + matCMemOffset; 
                    conditionalStoreAndLoad(matCBlockReg2, matCMemAddr2, 
                    fromMi2.matCMaxRegs, fromMi2.matRegToMemAddr, fromMp2);
                } else {
                    matCBlockReg2 = fromMi2.regMap[matCBlockReg2];
                }

                // REGMAP
                if (toMi.regMap.size() == 0) {
                    int toMatCMemAddr = toMi.matCMemStart + matCMemOffset; 
                    conditionalStoreAndLoad(toMatCBlockReg, toMatCMemAddr, 
                    toMi.matCMaxRegs, toMi.matRegToMemAddr, toMp);
                } else {
                    toMatCBlockReg = toMi.regMap[toMatCBlockReg];
                }      

                sumBlock(
                    fromMp1, fromMp2, vp, toMp,
                    fromCoreIdx1, fromCoreIdx2, VEC_CORE_START_IDX + toCoreIdx, toCoreIdx,
                    matCBlockReg1, matCBlockReg2,  
                    toMi.vecReg0, toMi.vecReg1, toMi.vecReg2, toMatCBlockReg
                );
            }
        }


        // write all valid matC regs to mem
        // not needed if called from ochestrator
        if (toMi.regMap.size() == 0) {
            MatCoreInst matInst;
            // TODO optimization no need to write all?
            for (int i = 0; i < toMi.matCMaxRegs; i++) {
                int matCReg = toMi.matCRegStart + i;
                int addr = toMi.matRegToMemAddr[matCReg];
                if (addr != -1) {
                    matInst.opcode = MatCoreInstDefn::STORE_MAT;
                    matInst.operands[MatCoreInstDefn::ADDR] = addr; 
                    matInst.operands[MatCoreInstDefn::M1] = matCReg; 
                    toMp.append(matInst);
                }
            }
        }
    }
}

std::tuple<int, int> getCoreAssignment(int matASize, int matA0Size) {
    // divide mats by number of cores
    int largeFactor= 2;
    int smallFactor = 2;
    assert(largeFactor * smallFactor == NUM_MAT_CORES);
    
    // TODO optimization compare row/col of matB vs matA for better division

    // short wide matA
    int coresForRows = smallFactor;
    int coresForCols = largeFactor;

    if (matASize > matA0Size) {
        // tall narrow matA
        coresForRows = largeFactor;
        coresForCols = smallFactor; 
    }
    return {coresForRows, coresForCols};
}

void gatherMatViaReg2(
    const int sendCoreIdx,
    const int recvCoreIdx,
    MatCoreProgram &sendProg,
    MatCoreProgram &recvProg,
    const std::vector<std::vector<size_t>> recvMatReg,
    const int matRegStart,
    const int matMaxRegs,
    const int matMemStart,
    int (&matRegToMemAddr)[MAT_REG_SIZE],
    const std::vector<size_t> &regMap
    ) {
    for (size_t bx = 0;bx < recvMatReg.size();bx++) {
        for (size_t by = 0;by < recvMatReg[0].size();by++) {
            int sendRegOffset = bx * recvMatReg.size() + by;
            int sendReg = matRegStart + sendRegOffset % matMaxRegs;
            if (regMap.size() == 0) {
                // benchmark
                
            } else {
                // orchestrator
                sendReg = regMap[sendReg];
            }

            if (sendCoreIdx == recvCoreIdx) {
                sendProg.append({MatCoreInstDefn::COPY, {
                    {MatCoreInstDefn::M1, sendReg},
                    {MatCoreInstDefn::Md, recvMatReg[bx][by]}
                }});
            } else {
                for (size_t r = 0;r < BLOCK_WIDTH;r++) {
                    // Send
                    sendProg.append({MatCoreInstDefn::SEND_ROW, {
                        {MatCoreInstDefn::CORE_IDX, recvCoreIdx},
                        {MatCoreInstDefn::M1, sendReg},
                        {MatCoreInstDefn::ROW_IDX, r}
                    }});
                    // Recv
                    recvProg.append({MatCoreInstDefn::RECV_ROW, {
                        {MatCoreInstDefn::CORE_IDX, sendCoreIdx},
                        {MatCoreInstDefn::M1, recvMatReg[bx][by]},
                        {MatCoreInstDefn::ROW_IDX, r}
                    }});
                }
            }
        }
    }
}

void multiCore(
    matrix &matA, 
    matrix &matB,
    matrix &matC,
    matrix &matRef) {

    auto [coresForRows, coresForCols] = getCoreAssignment(matA.size(), matA[0].size());

    std::vector<std::vector<matrix>> subMatsA = getSubMats(matA, coresForRows, coresForCols);     
    std::vector<std::vector<matrix>> subMatsB = getSubMats(matB, coresForRows, coresForCols);     
    std::vector<std::vector<matrix>> subMatsC = getSubMats(matC, coresForRows, coresForCols);     

    MatCoreProgram matProgs[NUM_MAT_CORES];
    VecCoreProgram vecProgs[NUM_VEC_CORES];
    std::vector<MatInfo> subMatInfos1;
    std::vector<MatInfo> subMatInfos2;

    /* MULT & ADD 1 (only support 4 cores for now)
        MULT       ADD
    core  A  B
        0: 00 00 --> vec0 -> mat0  
        1: 01 10 /
        2: 10 01 \
        3: 11 11 --> vec3 -> mat3
    */
    /* MULT & ADD 2 (only support 4 cores for now)
          MULT       ADD
     core  A  B
        0: 00 01 \   
        1: 01 11 --> vec1 -> mat1
        2: 10 00 --> vec2 -> mat2
        3: 11 10 / 
    */
    for (int i = 0; i < coresForRows; i++) {
        for (int j = 0; j < coresForCols; j++) {
            int matCoreOffset = i * coresForCols + j;
            int matCoreIdx = MAT_CORE_START_IDX + matCoreOffset;
            
            matrix subMatA = subMatsA[i][j];
            matrix subMatB; 
            matrix subMatC;

            // MULT & ADD 1 data (HARD CODED)
            if (matCoreOffset == 1 || matCoreOffset == 2) {
                subMatB = subMatsB[j][i];
                subMatC = subMatsC[i][i];
            } else {
                subMatB = subMatsB[i][j];
                subMatC = subMatsC[i][j];
            }
            subMatInfos1.push_back(
                MatInfo(subMatA, subMatB, subMatC)
            );

            // MULT & ADD 2 data (HARD CODED)
            switch (matCoreIdx) {
                case 0: {
                    subMatB = subMatsB[0][1];
                    subMatC = subMatsC[0][1];
                    break;
                }
                case 1: {
                    subMatB = subMatsB[1][1];
                    subMatC = subMatsC[0][1];
                    break;
                }
                case 2: {
                    subMatB = subMatsB[0][0];
                    subMatC = subMatsC[1][0];
                    break;
                }
                case 3: {
                    subMatB = subMatsB[1][0];
                    subMatC = subMatsC[1][0];
                    break;
                }
            }

            subMatInfos2.push_back(
                MatInfo(subMatA, subMatB, subMatC)
            );

            subMatInfos2[matCoreIdx].matAMemStart = 0;
            subMatInfos2[matCoreIdx].matBMemStart = subMatInfos1[matCoreIdx].matAMemSize + 
                subMatInfos1[matCoreIdx].matBMemSize + subMatInfos1[matCoreIdx].matCMemSize;
            subMatInfos2[matCoreIdx].matCMemStart = 
                subMatInfos2[matCoreIdx].matBMemStart + subMatInfos2[matCoreIdx].matBMemSize;
        }
    }

    for (int i = 0; i < coresForRows; i++) {
        for (int j = 0; j < coresForCols; j++) {
            int matCoreOffset = i * coresForCols + j;
            int vecCoreOffset = i * coresForCols + j;
            int matCoreIdx = MAT_CORE_START_IDX + matCoreOffset;
            int vecCoreIdx = VEC_CORE_START_IDX + vecCoreOffset;
 
            std::ofstream matDM(getDataMemName(matCoreIdx));
            matDM << std::setprecision(FLOAT_PRECISION) << std::fixed;

            MatInfo &mi1 = subMatInfos1[matCoreIdx];
            MatInfo &mi2 = subMatInfos2[matCoreIdx];

            // MULT & ADD 1 
            for (int rBlockIdx = 0; rBlockIdx < mi1.matARBlockSize; rBlockIdx++) {
                for (int cBlockIdx = 0; cBlockIdx < mi1.matACBlockSize; cBlockIdx++) {
                    writeBlockMatToMem(mi1.matA, rBlockIdx, cBlockIdx, true, matDM);
                }
            }
            
            for (int rBlockIdx = 0; rBlockIdx < mi1.matBRBlockSize; rBlockIdx++) {
                for (int cBlockIdx = 0; cBlockIdx < mi1.matBCBlockSize; cBlockIdx++) {
                    writeBlockMatToMem(mi1.matB, rBlockIdx, cBlockIdx, false, matDM);
                }
            }

            for (int rBlockIdx = 0; rBlockIdx < mi1.matARBlockSize; rBlockIdx++) {
                for (int cBlockIdx = 0; cBlockIdx < mi1.matBCBlockSize; cBlockIdx++) {
                    writeBlockMatToMem(mi1.matC, rBlockIdx, cBlockIdx, false, matDM);
                }
            }

            // MULT & ADD 2
            for (int rBlockIdx = 0; rBlockIdx < mi2.matBRBlockSize; rBlockIdx++) {
                for (int cBlockIdx = 0; cBlockIdx < mi2.matBCBlockSize; cBlockIdx++) {
                    writeBlockMatToMem(mi2.matB, rBlockIdx, cBlockIdx, false, matDM);
                }
            }

            for (int rBlockIdx = 0; rBlockIdx < mi2.matARBlockSize; rBlockIdx++) {
                for (int cBlockIdx = 0; cBlockIdx < mi2.matBCBlockSize; cBlockIdx++) {
                    writeBlockMatToMem(mi2.matC, rBlockIdx, cBlockIdx, false, matDM);
                }
            }

            matDM.close();

            std::ofstream vecDM(getDataMemName(vecCoreIdx));
            vecDM.close();
        }
    }
    // fromCoreIdx1, fromCoreIdx2, toCoreIdx
    std::vector<std::tuple<int, int, int>> addCoreIdxs1{{0, 1, 0}, {2, 3, 3}};
    multiMultAndAdd(coresForRows, coresForCols, subMatInfos1, matProgs, vecProgs, addCoreIdxs1);
    
    // TODO optimization: exchange data, 
    // for now we let the compiler store all required data in advance
    // to avoid exchanging
    /* EXCHANGE subMatB
        0 (00) <-> 2 (10)
        1 (01) <-> 3 (11)
    */
    std::vector<std::tuple<int, int, int>> addCoreIdxs2{{0, 1, 1}, {2, 3, 2}};
    multiMultAndAdd(coresForRows, coresForCols, subMatInfos2, matProgs, vecProgs, addCoreIdxs2);    

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
