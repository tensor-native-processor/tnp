#ifndef __TENSOR_H__
#define __TENSOR_H__

#include "onnx.proto3.pb.h"

#include "orchestration.h"

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

    // Name (optional)
    std::string m_name;

    // Dimension
    Shape m_shape;
    size_t m_size;

    // Value
    std::vector<float> m_value;

    // Locate a float from index
    float& locate(const Index&);
    const float& locate(const Index&) const;

    // Cast this tensor through unidirectional broadcast
    void unidirectionalBroadcast(const Shape&);

    static void multidirectionalBroadcast(std::initializer_list<std::reference_wrapper<Tensor>>);

    // Convert to Orchestrator::MatrixConstant
    Orchestrator::MatrixConstant toMatrixConstant(size_t) const;

private:
    void swap(Tensor&);
    void unidirectionalBroadcastCopy(size_t, Index&, Tensor&);
};


#endif
