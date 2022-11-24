#include "common.h"

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

    std::ofstream dm("data_mem.txt");
    for (int i = 0; i < matA.size(); ++i) {
        for (int j = 0; j < matA[i].size(); ++j) {
            dm << matA[i][j] << std::endl;
        }
    }
    for (int i = 0; i < matB.size(); ++i) {
        for (int j = 0; j < matB[i].size(); ++j) {
            dm << matB[i][j] << std::endl;
        }
    }
    dm.close();

}