#ifndef __MODEL_H__
#define __MODEL_H__

#include "onnx.proto3.pb.h"

#include <string>

// Typedefs for model
typedef ::onnx::GraphProto ONNXGraph;
typedef ::onnx::NodeProto ONNXNode;

// ONNX Model
class ONNXModel {
public:
    ONNXModel(const std::string&);

private:
    ::onnx::ModelProto m_model;
};


#endif
