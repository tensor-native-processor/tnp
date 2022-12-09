#include "orchestration.h"
#include "tensor.h"

#include <cstring>
#include <iomanip>
#include <fstream>

int main() {
    Orchestrator orch(OrchestratorParam{
        .width = 16,
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

    // Comparsion output
    std::ofstream ans("ans.txt");
    ans << std::setprecision(6) << std::fixed;
    for (size_t j = 0;j < m;j++) {
        for (size_t i = 0;i < n;i++) {
            ans << d1.locate({i, j}) << "\n";
        }
    }
    ans.close();


    // Call orchestrator
    auto mc1 = d1.toMatrixConstant(16);
    auto handle1 = orch.dataMatrixAllocate(mc1.m_shape);
    orch.dataMatrixLoadConstant(handle1, mc1);

    auto handle2 = orch.arithmeticTranspose(handle1);

    auto res = orch.dataMatrixStoreResult(handle2);

    orch.compile();

    return 0;
}
