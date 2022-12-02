#include "model.h"
#include "tensor.h"
#include <fstream>

int main() {
    ONNXModel model("618.onnx");

    // MNIST input
    Tensor mnistInputTensor({10, 400});
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
    for (size_t b = 0;b < mnistOutputTensor.m_shape[0];b++) {
        for (size_t i = 0;i < mnistOutputTensor.m_shape[1];i++) {
            std::cout << mnistOutputTensor.locate({b, i}) << ", ";
        }
        std::cout << "\n";
    }

    return 0;
}
