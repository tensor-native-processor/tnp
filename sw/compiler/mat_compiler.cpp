#include "common.h"
#include "mat_program.h"
#include "vec_program.h"

void single_core() {
       
}

int writeBlockMatToMem(std::vector<std::vector<float>> &mat, int rBlockIdx, int cBlockIdx, bool xflipTranspose, std::ofstream &dm) {
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
    // returns the mem offset
    return BLOCK_WIDTH * BLOCK_WIDTH;
}

int main(int argc, char *argv[]) {
    StartupOptions options = parseOptions(argc, argv);

    std::vector<std::vector<float>> matA;
    loadFromFile(options.matAFile, matA);

    std::vector<std::vector<float>> matB;
    loadFromFile(options.matBFile, matB);

    std::vector<std::vector<float>> matRef;
    loadFromFile(options.matRefFile, matRef);


    // compareMatrices(matBruteForce, matRef);

    matA.clear();
    for (int i = 0; i < 3; i ++) {
        std::vector<float> row;
        for (int j = 0; j < 3; j ++) {
           row.push_back(i * 3 + j + 1); 
        }
        matA.push_back(row);
    }
    printf("%d\n", matA.size());
    // for (int i = 0; i < 31; i++) {
    //     std::vector<float> row;
    //     for (int j = 0; j < 31; j++) {
    //        row.push_back(j + 1); 
    //     }
    //     matA.push_back(row);
    // }

    matB.clear();
    for (int i = 0; i < 3; i ++) {
        std::vector<float> row;
        for (int j = 0; j < 3; j ++) {
           row.push_back(i * 3 + j + 1); 
        }
        matB.push_back(row);
    }
    // for (int i = 0; i < 31; i++) {
    //     std::vector<float> row;
    //     for (int j = 0; j < 17; j++) {
    //        row.push_back(j); 
    //     }
    //     matB.push_back(row);
    // }
    
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

    printf("%d %d %d %d\n", matARSize, matACSize, matARBlockSize, matACBlockSize);

    std::vector<std::vector<float>> matBruteForce = multBruteForce(matA, matB); 
    std::ofstream ans("ans.txt");
    for (int i = 0; i < matARBlockSize * BLOCK_WIDTH; i++) {
        for (int j = 0; j < matBCBlockSize * BLOCK_WIDTH; j++) {
            float num = 0;
            if (i < matBruteForce.size() && j < matBruteForce[0].size()) {
                num = matBruteForce[i][j];
            }
            ans << num << std::endl;
        }
    }    
    ans.close();
    
    std::ofstream matDM("data_mem0.txt");
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
    
    std::ofstream vecDM("data_mem4.txt");
    vecDM.close(); 

    int matAMemSize = matARBlockSize * matACBlockSize * BLOCK_AREA;
    int matBMemSize = matBRBlockSize * matBCBlockSize * BLOCK_AREA;
    int matCMemSize = matARBlockSize * matBCBlockSize * BLOCK_AREA;

    int matAMemStart = 0;
    int matBMemStart = matAMemSize;
    int matCMemStart = matBMemStart + matBMemSize;

    // mat core registers
    int matRegs = (MAT_REG_SIZE - 1) / 3;
    int matARegStart = 0;
    int matBRegStart = matRegs;
    int matCRegStart = 2 * matRegs;
    // mat core tmp register
    int tmpReg = MAT_REG_SIZE - 1;

    // vec core registers
    int vecReg0 = 0;
    int vecReg1 = 1;
    int vecReg2 = 2;

    MatCoreProgram matProg;
    MatCoreInst matInst;

    VecCoreProgram vecProg;
    VecCoreInst vecInst;

    bool isMatARegsFull = false;
    for (int rBlockIdx = 0; rBlockIdx < matARBlockSize; rBlockIdx++) {
        if (isMatARegsFull) break;
        for (int kBlockIdx = 0; kBlockIdx < matACBlockSize; kBlockIdx++) {
            int matABlockOffset = rBlockIdx * matACBlockSize + kBlockIdx;
            int matAMemOffset = matABlockOffset * BLOCK_AREA;

            if (matABlockOffset == matRegs) {
                isMatARegsFull = true;
                break;
            }

            // load a matA block 
            matInst.opcode = MatCoreInstDefn::LOAD_MAT;
            matInst.operands[MatCoreInstDefn::ADDR] = matAMemStart + matAMemOffset;
            matInst.operands[MatCoreInstDefn::M1] = matARegStart + matABlockOffset;
            matProg.append(matInst);
        }
    }

    bool isMatBRegsFull = false;
    for (int kBlockIdx = 0; kBlockIdx < matACBlockSize; kBlockIdx++) {
        if (isMatBRegsFull) break;
        for (int cBlockIdx = 0; cBlockIdx < matBCBlockSize; cBlockIdx++) {
            int matBBlockOffset = kBlockIdx * matBCBlockSize + cBlockIdx;
            int matBMemOffset = matBBlockOffset * BLOCK_AREA;
            
            if (matBBlockOffset == matRegs) {
                isMatBRegsFull = true;
                break;
            }

            // load a matB block 
            matInst.opcode = MatCoreInstDefn::LOAD_MAT;
            matInst.operands[MatCoreInstDefn::ADDR] = matBMemStart + matBMemOffset; 
            matInst.operands[MatCoreInstDefn::M1] = matBRegStart + matBBlockOffset;
            matProg.append(matInst);
        }
    }
    
    bool isMatCRegsFull = false;
    for (int rBlockIdx = 0; rBlockIdx < matARBlockSize; rBlockIdx++) {
        if (isMatCRegsFull) break;
        for (int cBlockIdx = 0; cBlockIdx < matBCBlockSize; cBlockIdx++) {
            int matCBlockOffset = rBlockIdx * matACBlockSize + cBlockIdx;
            int matCMemOffset = matCBlockOffset * BLOCK_AREA;
            
            if (matCBlockOffset == matRegs) {
                isMatCRegsFull = true;
                break;
            }

            // load a matC block 
            matInst.opcode = MatCoreInstDefn::LOAD_MAT;
            matInst.operands[MatCoreInstDefn::ADDR] = matCMemStart + matCMemOffset; 
            matInst.operands[MatCoreInstDefn::M1] = matCRegStart + matCBlockOffset;
            matProg.append(matInst);
        }
    }

    // matCBlock_rc += matABlock_rk * matBBlock_kc
    for (int rBlockIdx = 0; rBlockIdx < matARBlockSize; rBlockIdx++) {
        for (int kBlockIdx = 0; kBlockIdx < matACBlockSize; kBlockIdx++) {
            int matABlockOffset = rBlockIdx * matACBlockSize + kBlockIdx;
            int matAMemOffset = matABlockOffset * BLOCK_AREA;
            int matABlockReg = matARegStart + matABlockOffset % matRegs;

            // TODO capacity miss (hash map?)
            if (matABlockOffset >= matRegs) {
                matInst.opcode = MatCoreInstDefn::LOAD_MAT;
                matInst.operands[MatCoreInstDefn::ADDR] = matAMemStart + matAMemOffset;
                matInst.operands[MatCoreInstDefn::M1] = matABlockReg;
                matProg.append(matInst);
            }

            for (int cBlockIdx = 0; cBlockIdx < matBCBlockSize; cBlockIdx++) {
                int matBBlockOffset = kBlockIdx * matBCBlockSize + cBlockIdx;
                int matBMemOffset = matBBlockOffset * BLOCK_AREA;
                int matBBlockReg = matBRegStart + matBBlockOffset % matRegs;
                
                // TODO capacity miss
                if (matBBlockOffset >= matRegs) {
                    matInst.opcode = MatCoreInstDefn::LOAD_MAT;
                    matInst.operands[MatCoreInstDefn::ADDR] = matBMemStart + matBMemOffset; 
                    matInst.operands[MatCoreInstDefn::M1] = matBBlockReg;
                    matProg.append(matInst);
                }
                
                int matCBlockOffset = rBlockIdx * matACBlockSize + cBlockIdx; 
                int matCMemOffset = matCBlockOffset * BLOCK_AREA;
                int matCBlockReg = matCRegStart + matCBlockOffset % matRegs;

                // TODO capacity miss
                if (matCBlockOffset >= matRegs) {
                    // must write back result first
                    // TODO not sure which ADDR it is... must keep prev?
                    matInst.opcode = MatCoreInstDefn::LOAD_MAT;
                    matInst.operands[MatCoreInstDefn::ADDR] = matCMemStart + matCMemOffset; 
                    matInst.operands[MatCoreInstDefn::M1] = matCBlockReg;
                    matProg.append(matInst);
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

                printf("matABlock_%d%d: %3d matBBlock_%d%d: %3d+%3d matCBlock_%d%d: %3d+%3d\n", 
                rBlockIdx, kBlockIdx, matAMemOffset, 
                kBlockIdx, cBlockIdx, matBMemStart, matBMemOffset,
                rBlockIdx, cBlockIdx, matCMemStart, matCMemOffset);
            }
        }
    }

    // store final result to memory
    // TODO capacity miss
    for (int rBlockIdx = 0; rBlockIdx < matARBlockSize; rBlockIdx++) {
        for (int cBlockIdx = 0; cBlockIdx < matBCBlockSize; cBlockIdx++) {
            int matCBlockOffset = rBlockIdx * matACBlockSize + cBlockIdx; 
            int matCMemOffset = matCBlockOffset * BLOCK_AREA;
            int matCBlockReg = matCRegStart + matCBlockOffset % matRegs;

            matInst.opcode = MatCoreInstDefn::STORE_MAT;
            matInst.operands[MatCoreInstDefn::ADDR] = matCMemStart + matCMemOffset;
            matInst.operands[MatCoreInstDefn::M1] = matCRegStart + matCBlockOffset;
            matProg.append(matInst);
        }
    }

    // halt mat
    matInst.opcode = MatCoreInstDefn::HALT;
    matProg.append(matInst);
    SaveProgram(matProg.toBinary(), "inst_mem0.txt");
    
    std::ofstream matTxt("inst_mem0_text.txt");
    matTxt << matProg.toText();
    matTxt.close();

    // halt vec    
    vecInst.opcode = VecCoreInstDefn::HALT;
    vecProg.append(vecInst);
    SaveProgram(vecProg.toBinary(), "inst_mem4.txt");

    std::ofstream vecTxt("inst_mem4_text.txt");
    vecTxt << vecProg.toText();
    vecTxt.close();

    // inst, data mem for mat core 1-3
    for (int i = 1; i < 4; i++) {
        MatCoreProgram prog;
        MatCoreInst inst; 
        inst.opcode = MatCoreInstDefn::HALT;
        prog.append(inst);
        char* fileName = (char*)malloc(14 * sizeof(char));
        snprintf(fileName, 14, "inst_mem%d.txt", i);
        SaveProgram(
            prog.toBinary(), fileName 
        );
        snprintf(fileName, 14, "data_mem%d.txt", i);
        std::ofstream dm(fileName);
        dm.close();
    }

    // inst, data mem for vec core 5-7
    for (int i = 5; i < 8; i++) {
        VecCoreProgram prog;
        VecCoreInst inst; 
        inst.opcode = VecCoreInstDefn::HALT;
        prog.append(inst);
        char* fileName = (char*)malloc(14 * sizeof(char));
        snprintf(fileName, 14, "inst_mem%d.txt", i);
        SaveProgram(
            prog.toBinary(), fileName 
        );
        snprintf(fileName, 14, "data_mem%d.txt", i);
        std::ofstream dm(fileName);
        dm.close();
    }

    return 0;
}