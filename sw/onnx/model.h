#ifndef __MODEL_H__
#define __MODEL_H__

#include "onnx.proto3.pb.h"

#include <string>
#include <vector>

// Shape
typedef std::vector<size_t> Shape;

// Tensor
class Tensor {
public:
    Tensor(const ::onnx::TensorProto&);
    Tensor(const Shape&);

    Tensor(const Tensor&);
    ~Tensor();

    // Name (optional)
    std::string m_name;

    // Dimension
    Shape m_shape;
    size_t m_size;

    // Value
    float* m_value;
};


// ONNX Model
class ONNXModel {
public:
    ONNXModel(const std::string&);

    void simulate();

private:
    // Protobuf
    ::onnx::ModelProto m_model;
    ::onnx::GraphProto m_graph;

    // Load model
    void loadModel();
    void loadModelInitializers();
    void genShape();

    // Data
    std::map<std::string, Tensor> m_initializers;
    std::map<std::string, Shape> m_inputs;
    std::vector<std::string> m_outputs;
};


#endif
