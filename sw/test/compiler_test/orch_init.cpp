#include "orchestration.h"

int main() {
    Orchestrator orch(OrchestratorParam{
        .width = 16,
        .matCacheSize = 256,
        .matCoreCount = 4,
        .vecCacheSize = 256,
        .vecCoreCount = 4
    });
    orch.compile();

    return 0;
}
