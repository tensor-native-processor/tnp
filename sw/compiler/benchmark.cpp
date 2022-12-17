#include <vector>

#include "compiler_common.h"
#include "mat_compiler.h"

int main(int argc, char *argv[]) {
    std::vector<std::vector<float>> matA;
    std::vector<std::vector<float>> matB;

    /*
    (31, 31) * (31, 17) took 2226 cycles (CPU time 49.670s)
    (64, 128) * (128, 10) single 8610 cycles (CPU time 178.530s) multi 5164 cycles (CPU time 429s)
    (64, 128) * (128, 10) took 8808 cycles if set MAT_REG_SIZE to 64 (CPU time 180.360s)
    (256, 256) * (256, 256) multi 279812 single 1154062
    */
    // 64 * 128
    for (int i = 0; i < 64; i++) {
        std::vector<float> row;
        for (int j = 0; j < 128; j++) {
            int num = j + 1;
            if (i >= 8) num *= 2;
            // float num = float(rand() % 65536 - 65536 / 2) / (65536 / 2);
            row.push_back(num); 
        }
        matA.push_back(row);
    }
    
    // 128 * 10
    for (int i = 0; i < 128; i++) {
        std::vector<float> row;
        for (int j = 0; j < 10; j++) {
            int num = j + 1;
            if (i >= 8) num *= 2;
            // float num = float(rand() % 65536 - 65536 / 2) / (65536 / 2);
            row.push_back(num); 
        }
        matB.push_back(row);
    }
    
    assert(matA[0].size() == matB.size() && "matA[0].size() != matB.size()");

    // init matC with zeroes
    std::vector<std::vector<float>> matC(matA.size(), std::vector<float>(matB[0].size(), 0));
    std::vector<std::vector<float>> matRef = multBruteForce(matA, matB); 

    bool useSingleCore = false;

    if (useSingleCore) {
        singleCore(matA, matB, matC, matRef); 
    } else {
        multiCore(matA, matB, matC, matRef); 
    }   
    return 0;
}