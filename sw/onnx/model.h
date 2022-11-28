#ifndef __MODEL_H__
#define __MODEL_H__

#include "onnx.proto3.pb.h"

#include <string>
#include <vector>

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


// ONNX Node
struct ONNXNode {
    std::string name, op_type;
    std::vector<std::string> inputs, outputs;
    std::map<std::string, ::onnx::AttributeProto> attributes;
};


// ONNX Model
class ONNXModel {
public:
    ONNXModel(const std::string&);

private:
    // Protobuf
    ::onnx::ModelProto m_model;
    ::onnx::GraphProto m_graph;

    // Graph of node dependents
    size_t m_nodeCount;
    std::vector<ONNXNode> m_nodeList;

    // Load model
    void loadModel();
    void loadModelInitializers();
    void genDepGraph();

    // Data
    std::map<std::string, Tensor> m_initializers;
};


#endif
