#ifndef COMPILER_COMMON_H_
#define COMPILER_COMMON_H_

#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

typedef std::vector<std::vector<float>> matrix;

struct StartupOptions {
    std::string matAFile;
    std::string matBFile;
    std::string matRefFile;
};

const float EPSILON = 1e-5;
const int BLOCK_WIDTH = 16;
const int BLOCK_AREA = BLOCK_WIDTH * BLOCK_WIDTH;
const int MAT_REG_SIZE = 256; //4;
const int VEC_REG_SIZE = 256; //4;
const int FLOAT_PRECISION = 6;
const int DATA_MEM_SIZE = 65536;
const int MAT_CORE_START_IDX = 0;
const int NUM_MAT_CORES = 4;
const int VEC_CORE_START_IDX = 4;
const int NUM_VEC_CORES = 4;

class MatInfo {
    public: 
        MatInfo(const matrix &matA, const matrix &matB, const matrix &matC);
        MatInfo(const matrix &matA, const matrix &matB, const matrix &matC, 
        int matAMemOffset, int matBMemOffset, int matCMemOffset);
        MatInfo(
            // These registers are just used to calculate the matrix shapes
            const std::vector<std::vector<size_t>> &m1Reg,
            const std::vector<std::vector<size_t>> &m2Reg,
            const std::vector<std::vector<size_t>> &m3Reg,
            // These registers are the actual ones used
            const std::vector<size_t> &freeRegIdx,
            int dataMemStart
        );

        matrix matA;
        matrix matB;
        matrix matC;

        int matARBlockSize;
        int matACBlockSize;
        int matBRBlockSize;
        int matBCBlockSize;
        
        int matAMemSize;
        int matBMemSize;
        int matCMemSize;
        
        int matAMemStart;
        int matBMemStart;
        int matCMemStart;
        
        int matAMaxRegs;
        int matBMaxRegs;
        int matCMaxRegs;

        int matARegStart;
        int matBRegStart;
        int matCRegStart;

        int tmpReg;

        int matRegToMemAddr[MAT_REG_SIZE];

        int vecReg0;
        int vecReg1;
        int vecReg2;

        std::vector<size_t> regMap;

    private:
        void init(const matrix &matA, const matrix &matB, const matrix &matC);
};

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

    std::vector<std::vector<float>> ans(matA.size(), std::vector<float>(matB[0].size(), 0));
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

inline std::string getDataMemName(int coreIdx) {
    return "data_mem" + std::to_string(coreIdx) + ".txt"; 
}

inline std::string getInstMemName(int coreIdx) {
    return "inst_mem" + std::to_string(coreIdx) + ".txt"; 
}

inline std::string getInstMemTextName(int coreIdx) {
    return "inst_mem" + std::to_string(coreIdx) + "_text.txt"; 
}

inline std::string getAnsName(int coreIdx) {
    return "ans" + std::to_string(coreIdx) + ".txt"; 
}

#endif
