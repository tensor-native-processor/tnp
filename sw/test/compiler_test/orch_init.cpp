#include "orchestration.h"

#include <cstring>

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

    // Fill data into H3
    Orchestrator::MatrixConstant h3Data;
    h3Data.m_shape = {2, 2};
    h3Data.m_data.assign(2,
        std::vector<std::vector<float>>(2,
            std::vector<float>(256, 0)
        )
    );
    h3Data.m_data[0][0][3 * 16 + 4] = 9;
    h3Data.m_data[1][0][2 * 16 + 2] = 12;
    orch.dataMatrixLoadConstant(h3, h3Data);

    orch.compile();

    return 0;
}
