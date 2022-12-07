#include "model.h"
#include "orchestration.h"
#include "tensor.h"
#include <fstream>

int main() {
    ONNXModel model("618.onnx");

    // MNIST input
    Tensor mnistInputTensor({64, 400});
    std::ifstream mnistInputFile("X.txt");
    for (size_t b = 0;b < mnistInputTensor.m_shape[0];b++) {
        for (size_t i = 0;i < mnistInputTensor.m_shape[1];i++) {
            mnistInputFile >> mnistInputTensor.locate({b, i});
        }
    }
    mnistInputFile.close();

    // Run simulation
    auto mnistOutputTensors = model.simulate(std::vector<Tensor>{
        mnistInputTensor
    });

    // MNIST output
    const auto& mnistOutputTensor = mnistOutputTensors[0];
    /*
    for (size_t b = 0;b < mnistOutputTensor.m_shape[0];b++) {
        for (size_t i = 0;i < mnistOutputTensor.m_shape[1];i++) {
            std::cout << mnistOutputTensor.locate({b, i}) << ", ";
        }
        std::cout << "\n";
    }
    */
    for (size_t b = 0;b < mnistOutputTensor.m_shape[0];b++) {
        size_t index = 0;
        for (size_t i = 0;i < mnistOutputTensor.m_shape[1];i++) {
            if (mnistOutputTensor.locate({b, i}) > mnistOutputTensor.locate({b, index})) {
                index = i;
            }
        }
        for (size_t i = 0;i < mnistOutputTensor.m_shape[1];i++) {
            if (i != 0) {
                std::cout << " ";
            }
            if (i == index) {
                std::cout << "1.0";
            } else {
                std::cout << "0.0";
            }
        }
        std::cout << "\n";
    }

    // Run compilation
    model.compile(OrchestratorParam{
        .width = 16,
        .matCacheSize = 256,
        .matCoreCount = 4,
        .vecCacheSize = 256,
        .vecCoreCount = 4
    }, std::vector<Tensor>{
        mnistInputTensor
    });

    return 0;
}
