#include <vector>

#include "compiler_common.h"
#include "mat_compiler.h"

int main(int argc, char *argv[]) {
    std::vector<std::vector<float>> matA;
    std::vector<std::vector<float>> matB;
    
    int w = 64;
    bool useSingleCore = true;
    // useSingleCore = false;

    for (int i = 0; i < w; i++) {
        std::vector<float> row;
        for (int j = 0; j < w; j++) {
            float num = float(rand() % 65536 - 65536 / 2) / (65536 / 2);
            row.push_back(num); 
        }
        matA.push_back(row);
    }
    
    for (int i = 0; i < w; i++) {
        std::vector<float> row;
        for (int j = 0; j < w; j++) {
            float num = float(rand() % 65536 - 65536 / 2) / (65536 / 2);
            row.push_back(num); 
        }
        matB.push_back(row);
    }
    
    assert(matA[0].size() == matB.size() && "matA[0].size() != matB.size()");

    // init matC with zeroes
    std::vector<std::vector<float>> matC(matA.size(), std::vector<float>(matB[0].size(), 0));
    std::vector<std::vector<float>> matRef = multBruteForce(matA, matB); 

    if (useSingleCore) {
        singleCore(matA, matB, matC, matRef); 
    } else {
        multiCore(matA, matB, matC, matRef); 
    }
    
    return 0;
}