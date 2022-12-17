#include "orchestration.h"
#include "tensor.h"

#include <cstring>
#include <iomanip>
#include <iostream>
#include <fstream>

int main() {
    const size_t width = 16;
    Orchestrator orch(OrchestratorParam{
        .width = width,
        .matCacheSize = 256,
        .matCoreCount = 4,
        .vecCacheSize = 256,
        .vecCoreCount = 4
    });

    size_t n = 32, m = 32, o = 32;
    Tensor d1Orig({m, n});
    Tensor d1({n, m});
    Tensor d2({m, o});
    Tensor d3({n, o});

    std::vector<float> d1Vec;

    for (size_t i = 0;i < m;i++) {
        for (size_t j = 0;j < n;j++) {
            d1Orig.locate({i, j}) = rand() % 536 - 536 / 2; // (i + 1) * (j + 1);
        }
    }
    // block 00, 10, 01, 11 
    for (int i = 0; i < 2; i ++) {
        for (int j = 0; j < 2; j++) {
            int blockI = j;
            int blockJ = i;
            std::vector<float> block;
            for (int ii = i * 16; ii < (i + 1) * 16; ii++) {
                for (int jj = (j + 1) * 16 - 1; jj >= j * 16; jj--) {
                    std::cout << jj << " " << ii << ", ";
                    block.push_back(d1Orig.locate({size_t(jj), size_t(ii)}));
                }
                std:: cout << std::endl;
            }

            // fill in d1
            int idx = 0;
            for (int ii = blockI * 16; ii < (blockI + 1) * 16; ii++) {
                for (int jj = blockJ * 16; jj < (blockJ + 1) * 16; jj++) {
                    d1.locate({size_t(ii), size_t(jj)}) = block[idx];
                    idx += 1;
                }
            }
        }
    }

    for (size_t i = 0;i < m;i++) {
        for (size_t j = 0;j < o;j++) {
            d2.locate({i, j}) = rand() % 536 - 536 / 2; //j + 1;
        }
    }
    std::cout << "d1 after" << std::endl;
    for (size_t i = 0;i < n;i++) {
        for (size_t j = 0;j < m;j++) {
            std::cout << d1.locate({i, j}) << " ";
        }
        std::cout << std::endl;
    }

    // Call orchestrator
    auto mc1 = d1.toMatrixConstant(width);
    auto handle1 = orch.dataMatrixAllocate(mc1.m_shape);
    orch.dataMatrixLoadConstant(handle1, mc1);

    auto mc2 = d2.toMatrixConstant(width);
    auto handle2 = orch.dataMatrixAllocate(mc2.m_shape);
    orch.dataMatrixLoadConstant(handle2, mc2);
    
    auto handle3 = orch.arithmeticMatMult(handle1, handle2);

    orch.dataMatrixDeallocate(handle1);
    orch.dataMatrixDeallocate(handle2);
    
    auto res = orch.dataMatrixStoreResult(handle3);

    for (size_t i = 0;i < n;i++) {
        for (size_t j = 0;j < o;j++) {
            float num = 0;
            for (size_t k = 0; k < m; k++) {
                num += d1Orig.locate({i, k}) * d2.locate({k, j});
            }
            d3.locate({i, j}) = num;
        }
    } 
    auto mc3 = d3.toMatrixConstant(width);

    // Reference output
    std::ofstream ans("ans.txt");
    ans << std::setprecision(6) << std::fixed;
    for (size_t i = 0;i < mc3.m_shape.x * width;i++) {
        for (size_t j = 0;j < mc3.m_shape.y * width;j++) {
            if (i < n && j < o) {
                float x = d3.locate({i, j});
                ans << x << "\n";
            } else {
                ans << 0.0f << "\n";
            }
        }
    }
    ans.close();

    // Print hint to check output
    std::ofstream hint("hint.txt");
    hint << res.toHintLine(width);
    hint.close();

    orch.compile();

    return 0;
}