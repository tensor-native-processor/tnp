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
        .matCacheSize = 4096,
        .matCoreCount = 4,
        .vecCacheSize = 4096,
        .vecCoreCount = 4
    });

    size_t n = 56, m = 103;
    Tensor d1({n, m});

    for (size_t i = 0;i < n;i++) {
        for (size_t j = 0;j < m;j++) {
            d1.locate({i, j}) = rand() % 65536;
        }
    }

    // Call orchestrator
    auto mc1 = d1.toMatrixConstant(width);
    auto handle1 = orch.dataMatrixAllocate(mc1.m_shape);
    orch.dataMatrixLoadConstant(handle1, mc1);

    auto handle2 = orch.arithmeticTranspose(handle1);

    auto res = orch.dataMatrixStoreResult(handle2);


    // Reference output
    std::ofstream ans("ans.txt");
    ans << std::setprecision(6) << std::fixed;
    for (size_t j = 0;j < mc1.m_shape.y * width;j++) {
        for (size_t i = 0;i < mc1.m_shape.x * width;i++) {
            if (i < n && j < m) {
                ans << d1.locate({i, j}) << "\n";
            } else {
                ans << 0.0f << "\n";
            }
        }
    }
    ans.close();


    // Print hint to check output
    std::ofstream hint("hint.txt");
    std::cout << res.m_shape.x << ", " << res.m_shape.y << "\n";
    for (size_t i = 0;i < res.m_shape.x * width;i++) {
        for (size_t j = 0;j < res.m_shape.y * width;j++) {
            hint << res.m_dataAddr[i / width][j / width] + width * (i % width) + j % width << "\n";
        }
    }
    hint.close();

    orch.compile();

    return 0;
}
