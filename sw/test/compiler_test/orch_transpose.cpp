#include "orchestration.h"
#include "tensor.h"

#include <cstring>

int main() {
    Orchestrator orch(OrchestratorParam{
        .width = 16,
        .matCacheSize = 256,
        .matCoreCount = 4,
        .vecCacheSize = 256,
        .vecCoreCount = 4
    });

    size_t n = 56, m = 4038;
    Tensor d1({n, m});

    for (size_t i = 0;i < n;i++) {
        for (size_t j = 0;j < m;j++) {
            d1.locate({i, j}) = rand() % 256;
        }
    }


    auto mc1 = d1.toMatrixConstant(16);
    auto handle1 = orch.dataMatrixAllocate(mc1.m_shape);
    orch.dataMatrixLoadConstant(handle1, mc1);

    auto handle2 = orch.arithmeticTranspose(handle1);

    orch.compile();

    return 0;
}
