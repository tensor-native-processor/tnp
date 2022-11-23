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
}