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
    Tensor d1({n, m});
    Tensor d2({m, o});
    Tensor d3({n, o});

    for (size_t i = 0;i < n;i++) {
        for (size_t j = 0;j < m;j++) {
            d1.locate({i, j}) = j + 1; //rand() % 65536 - 65536 / 2;
        }
    }

    for (size_t i = 0;i < m;i++) {
        for (size_t j = 0;j < o;j++) {
            d2.locate({i, j}) = j + 1;// rand() % 65536 - 65536 / 2;
        }
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
                num += d1.locate({i, k}) * d2.locate({k, j});
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
            if (i < n && j < m) {
                float x = d3.locate({i, j});
                ans << (x > 0.0f ? x : 0.0f) << "\n";
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