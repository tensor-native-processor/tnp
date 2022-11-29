#include "common.h"
#include "mat_program.h"

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

    std::vector<std::vector<float>> matBruteForce = multBruteForce(matA, matB);

    compareMatrices(matBruteForce, matRef);

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

    int matARSize = matA.size();
    int matACSize = matA[0].size();
    int matBRSize = matB.size();
    int matBCSize = matB[0].size();
    // int matASize = matARSize * matACSize;
    // int matBSize = matBRSize * matBCSize;

    int matARBlockSize = (matA.size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
    int matACBlockSize = (matA[0].size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
    int matBRBlockSize = (matB.size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
    int matBCBlockSize = (matB[0].size() + BLOCK_WIDTH - 1) / BLOCK_WIDTH;

    printf("%d %d %d %d\n", matARSize, matACSize, matARBlockSize, matACBlockSize);

    std::ofstream dm("data_mem.txt");
    for (int rBlockIdx = 0; rBlockIdx < matARBlockSize; rBlockIdx++) {
        for (int cBlockIdx = 0; cBlockIdx < matACBlockSize; cBlockIdx++) {
            writeBlockMatToMem(matA, rBlockIdx, cBlockIdx, true, dm);
        }
    }
    
    for (int rBlockIdx = 0; rBlockIdx < matBRBlockSize; rBlockIdx++) {
        for (int cBlockIdx = 0; cBlockIdx < matBCBlockSize; cBlockIdx++) {
            writeBlockMatToMem(matB, rBlockIdx, cBlockIdx, false, dm);
        }
    }
    dm.close();

    int matAMemSize = matARBlockSize * matACBlockSize * BLOCK_AREA;
    int matBMemSize = matBRBlockSize * matBCBlockSize * BLOCK_AREA;

    MatCoreProgram prog;
    MatCoreInst inst;

    for (int rBlockIdx = 0; rBlockIdx < matARBlockSize; rBlockIdx++) {
        for (int kBlockIdx = 0; kBlockIdx < matACBlockSize; kBlockIdx++) {
            int matAMemOffset = rBlockIdx * matACBlockSize * BLOCK_AREA + kBlockIdx * BLOCK_AREA;
            // load a matA block to memory 
            inst.opcode = MatCoreInstDefn::LOAD_MAT;
            inst.operands[MatCoreInstDefn::ADDR] = matAMemOffset;
            inst.operands[MatCoreInstDefn::M1] = 0;
            prog.append(inst);

            for (int cBlockIdx = 0; cBlockIdx < matBCBlockSize; cBlockIdx++) {
                // matCBlock_rc += matABlock_rk * matBBlock_kc
                
                // load a matB block to memory
                int matBMemOffset = kBlockIdx * matBCBlockSize * BLOCK_AREA + cBlockIdx * BLOCK_AREA;
                inst.opcode = MatCoreInstDefn::LOAD_MAT;
                inst.operands[MatCoreInstDefn::ADDR] = matAMemSize + matBMemOffset; 
                inst.operands[MatCoreInstDefn::M1] = 1;
                prog.append(inst);
                
                // set weight of matA into systolic array
                inst.opcode = MatCoreInstDefn::SET_WEIGHT;
                inst.operands[MatCoreInstDefn::M1] = 0;
                prog.append(inst);

                // multiply
                inst.opcode = MatCoreInstDefn::MULTIPLY;
                inst.operands[MatCoreInstDefn::Md] = 2;
                inst.operands[MatCoreInstDefn::M1] = 1;
                prog.append(inst);

                // store result to memory
                int matCMemOffset = rBlockIdx * matACBlockSize * BLOCK_AREA + cBlockIdx * BLOCK_AREA; 
                inst.opcode = MatCoreInstDefn::STORE_MAT;
                inst.operands[MatCoreInstDefn::ADDR] = matAMemSize + matBMemSize;
                inst.operands[MatCoreInstDefn::M1] = 2;
                prog.append(inst);


                printf("matABlock_%d%d: %3d matBBlock_%d%d: %3d+%3d matCBlock_%d%d: %3d+%3d\n", 
                rBlockIdx, kBlockIdx, matAMemOffset, 
                kBlockIdx, cBlockIdx, matAMemSize, matBMemOffset,
                rBlockIdx, cBlockIdx, matAMemSize + matBMemSize, matCMemOffset);
            }
        }
    }
 
    return 0;
    // // write matrices to data mem
    // std::ofstream dm("data_mem.txt");
    // for (int i = 0; i < matARSize; i++) {
    //     for (int j = 0; j < matACSize; ++j) {
    //         dm << matA[i][j] << std::endl;
    //     }
    // }
    // for (int i = 0; i < matBRSize; i++) {
    //     for (int j = 0; j < matBCSize; ++j) {
    //         dm << matB[i][j] << std::endl;
    //     }
    // }
    // dm.close();

    /*

    // halt
    inst.opcode = MatCoreInstDefn::HALT;
    prog.append(inst);

    TNPProgramBinary bin = prog.toBinary();
    SaveProgram(bin, "inst_mem.txt");

    return 0;
    */
}