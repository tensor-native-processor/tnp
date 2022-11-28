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
    ~Tensor();

    // Name
    std::string m_name;

    // Dimension
    Shape m_shape;
    size_t m_size;

    // Value
    float* m_value;
};


// ONNX Node
class ONNXNode {
public:
    std::string m_name, m_op_type;
    std::vector<std::string> m_inputs, m_outputs;
    std::map<std::string, ::onnx::AttributeProto> m_attributes;
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
    void genShape();

    // Data
    std::map<std::string, Tensor> m_initializers;
    std::map<std::string, Shape> m_inputs;
    std::vector<std::string> m_outputs;
};


#endif
