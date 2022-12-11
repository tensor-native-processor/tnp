#ifndef __MODEL_H__
#define __MODEL_H__

#include "onnx.pb.h"

#include "tensor.h"
#include "orchestration.h"

#include <string>
#include <vector>


// ONNX Model
class ONNXModel {
public:
    ONNXModel(const std::string&);

    // Operations on the model
    std::vector<Tensor> simulate(const std::vector<Tensor>&);
    void compile(const OrchestratorParam&, const std::vector<Tensor>&);

private:
    // Protobuf
    ::onnx::ModelProto m_model;
    ::onnx::GraphProto m_graph;

    // Load model
    void loadModel();
    void loadModelInitializers();
    void inferShape();

    // Data
    std::map<std::string, Tensor> m_initializers;
    std::map<std::string, Tensor::Shape> m_inputs;
    std::vector<std::string> m_outputs;
};


#endif
