#ifndef __MODEL_H__
#define __MODEL_H__

#include "onnx.proto3.pb.h"

#include <string>
#include <vector>

// Typedefs for model
typedef ::onnx::GraphProto ONNXGraph;
typedef ::onnx::NodeProto ONNXNode;

// Tensor
class Tensor {
public:
    Tensor(const ::onnx::TensorProto&);
    ~Tensor();

private:
    // Name
    std::string m_name;

    // Dimension
    std::vector<size_t> m_dims;
    size_t m_size;

    // Value
    float* m_value;
};


// ONNX Model
class ONNXModel {
public:
    ONNXModel(const std::string&);

private:
    // Protobuf
    ::onnx::ModelProto m_model;
    ::onnx::GraphProto m_graph;

    // Load model
    void loadModel();
    void loadModelInitializers();

    // Data
    std::map<std::string, Tensor> m_initializers;
};


#endif
