#ifndef COMMON_H_
#define COMMON_H_

#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct StartupOptions {
    std::string matAFile;
    std::string matBFile;
    std::string matRefFile;
};

const float EPSILON = 1e-5;
const float CORE_MAT_SIZE = 128;

inline StartupOptions parseOptions(int argc, char *argv[]) {
    StartupOptions rs;
    for (int i = 1; i < argc; i++) {
        if (i < argc - 1) {
            if (strcmp(argv[i], "-ma") == 0)
                rs.matAFile = argv[i + 1];
            else if (strcmp(argv[i], "-mb") == 0)
                rs.matBFile = argv[i + 1];
            else if (strcmp(argv[i], "-mr") == 0)
                rs.matRefFile = argv[i + 1];
        }
    }
    return rs;
}

inline void printMatrix(std::vector<std::vector<float>> &matrix) {
    for (auto &row: matrix) {
        for (auto &num : row) {
            printf("%f ", num);
        }
        printf("\n");
    }
}

inline bool loadFromFile(std::string fileName,
                         std::vector<std::vector<float>> &matrix) {
    std::ifstream f(fileName);
    assert((bool)f && "Cannot open input file");

    std::string line;
    while (std::getline(f, line)) {
        std::istringstream iss(line);
        std::vector<float> row;
        float num;
        while (iss >> num) {
            row.push_back((num));
        }
        matrix.push_back(row);
    }
    return true;
}

inline std::vector<std::vector<float>> multBruteForce(const std::vector<std::vector<float>> &matA, const
std::vector<std::vector<float>> &matB) {
    assert(matA[0].size() == matB.size());

    std::vector<std::vector<float>> ans(matA.size(), std::vector<float>(matB.size()));
    for (int i = 0; i < ans.size(); i++) {
        for (int j = 0; j < ans[i].size(); j++) {
            for (int k = 0; k < matB.size(); k++) {
                ans[i][j] += matA[i][k] * matB[k][j];
            }
        }
    }
    return ans;
}

inline void compareMatrices(const std::vector<std::vector<float>> &matSource,
                       const std::vector<std::vector<float>> &matRef) {
    assert(matSource.size() == matRef.size());

    for (int i = 0; i < matSource.size(); ++i) {
        assert(matSource[i].size() == matRef[i].size());
        for (int j = 0; j < matSource[i].size(); ++j) {
            if (fabs(matSource[i][j] - matRef[i][j]) > EPSILON) {
                fprintf(stderr, "fabs(%.8f - %.8f) > %.6f\n", matSource[i][j], matRef[i][j], EPSILON);
                assert(false);
            }
        }
    }
}

#endif
