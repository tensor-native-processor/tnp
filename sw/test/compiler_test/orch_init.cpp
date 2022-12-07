#include "orchestration.h"

int main() {
    Orchestrator orch(OrchestratorParam{
        .width = 16,
        .matCacheSize = 256,
        .matCoreCount = 4,
        .vecCacheSize = 256,
        .vecCoreCount = 4
    });
    auto h1 = orch.dataMatrixAllocate({3, 4});
    auto h2 = orch.dataMatrixAllocate({8, 3});

    orch.dataMatrixDeallocate(h2);

    auto h3 = orch.dataMatrixAllocate({2, 2});

    orch.compile();

    return 0;
}
