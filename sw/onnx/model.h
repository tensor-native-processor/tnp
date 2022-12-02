#ifndef __MODEL_H__
#define __MODEL_H__

#include "onnx.proto3.pb.h"

#include <string>
#include <vector>


// Tensor
class Tensor {
public:
    // Types
    typedef std::vector<size_t> Index;
    typedef std::vector<size_t> Shape;

    // Constructor
    Tensor(const ::onnx::TensorProto&);
    Tensor(const Shape&);

    Tensor(const Tensor&);

    // Name (optional)
    std::string m_name;

    // Dimension
    Shape m_shape;
    size_t m_size;

    // Value
    std::unique_ptr<float[]> m_value;

    // Locate a float from index
    float& locate(const Index&);
    const float& locate(const Index&) const;

    // Cast this tensor through unidirectional broadcast
    void unidirectionalBroadcast(const Shape&);

private:
    void swap(Tensor&);
    void unidirectionalBroadcastCopy(size_t, Index&, Tensor&);
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
    std::map<std::string, Tensor::Shape> m_inputs;
    std::vector<std::string> m_outputs;
};


#endif
