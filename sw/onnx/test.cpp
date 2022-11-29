#include "model.h"

int main() {
    ONNXModel model("618.onnx");
    model.simulate();

    return 0;
}
