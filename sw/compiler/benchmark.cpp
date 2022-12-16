#include <vector>

#include "compiler_common.h"
#include "mat_compiler.h"

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