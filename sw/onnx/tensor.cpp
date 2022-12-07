#include "model.h"
#include "tensor.h"
#include "logging.h"
#include "error.h"
#include "operator.h"

#include <queue>
#include <fstream>

// Construct Tensor from protobuf
Tensor::Tensor(const ::onnx::TensorProto& tensor) {
    // Name
    m_name = tensor.name();

    // Dimension
    m_size = 1;
    for (auto dim : tensor.dims()) {
        m_shape.push_back(dim);
        m_size *= dim;
    }
    m_value.assign(m_size, 0.0f);

    // Value
    switch (tensor.data_type()) {
    case ::onnx::TensorProto::FLOAT: {
        if (tensor.float_data_size() != 0) {
            // Float data
            if ((size_t)tensor.float_data_size() > m_size) {
                FatalError("Initializer " + m_name + " larger than dimension " + std::to_string(m_size));
            }
            for (size_t i = 0;i < (size_t)tensor.float_data_size();i++) {
                m_value[i] = tensor.float_data(i);
            }
        } else {
            // Raw data
            std::string bytes = tensor.raw_data();
            if (bytes.size() / sizeof(float) > m_size) {
                FatalError("Initializer " + m_name + " larger than dimension " + std::to_string(m_size));
            }
            std::copy_n(bytes.c_str(), bytes.size(), (char*)m_value.data());
        }
        break;
    }
    case ::onnx::TensorProto::INT64: {
        if (tensor.int64_data_size() != 0) {
            if ((size_t)tensor.int64_data_size() > m_size) {
                FatalError("Initializer " + m_name + " larger than dimension " + std::to_string(m_size));
            }
            for (size_t i = 0;i < (size_t)tensor.int64_data_size();i++) {
                // Casted to float for now
                m_value[i] = (float)tensor.int64_data(i);
            }
        } else {
            LogWarning("Initializer tensor " + m_name + " int64 raw data");
        }
        break;
    }
    default: {
        LogWarning("Initializer tensor " + m_name + " has unsupported type " + std::to_string(tensor.data_type()) + " and set to 0");
    }
    }
}

// Construct Tensor from shape
Tensor::Tensor(const Shape& shape)
: m_shape(shape) {
    // Calculate size
    m_size = 1;
    for (auto dim : m_shape) {
        m_size *= dim;
    }
    // Allocate value
    m_value.assign(m_size, 0.0f);
}


// Locate an index in tensor
const float& Tensor::locate(const Index& idx) const {
    if (idx.size() != m_shape.size()) {
        FatalError("Tensor index mismatch shape");
    }

    size_t decode = 0, block = 1;
    for (ssize_t pos = m_shape.size() - 1;pos >= 0;pos--) {
        decode += idx[pos] * block;
        block *= m_shape[pos];
    }
    return m_value[decode];
}
float& Tensor::locate(const Index& idx) {
    return const_cast<float&>(const_cast<const Tensor*>(this)->locate(idx));
}

// Swap with another Tensor
void Tensor::swap(Tensor& x) {
    std::swap(m_name, x.m_name);
    std::swap(m_shape, x.m_shape);
    std::swap(m_size, x.m_size);
    std::swap(m_value, x.m_value);
}

// Unidirectional broadcast
// https://github.com/onnx/onnx/blob/main/docs/Broadcasting.md
void Tensor::unidirectionalBroadcast(const Shape& destShape) {
    size_t destDim = destShape.size();
    size_t deltaDim = destDim - m_shape.size();
    if (deltaDim < 0) {
        FatalError("Tensor u-broadcast dim inv");
    }
    // Prepend 1 to m_shape (m_size is the same)
    if (deltaDim > 0) {
        m_shape.insert(m_shape.begin(), deltaDim, 1);
    }

    // Validate shape
    bool sameShape = true;
    for (size_t d = 0;d < deltaDim;d++) {
        if (m_shape[d] != destShape[d]) {
            sameShape = false;
            if (m_shape[d] != 1) {
                FatalError("Tensor u-broadcast shape err");
            }
        }
    }
    if (sameShape) {
        return;
    }

    // Copy tensor
    Tensor outTensor(destShape);
    Index progIdx(destDim, 0);
    unidirectionalBroadcastCopy(0, progIdx, outTensor);
    swap(outTensor);
}
void Tensor::unidirectionalBroadcastCopy(size_t x, Index& progIdx, Tensor& outTensor) {
    if (x == progIdx.size()) {
        // Copy vector
        Index origIdx = progIdx;
        for (size_t d = 0;d < progIdx.size();d++) {
            if (origIdx[d] >= m_shape[d]) {
                // Override shape
                if (m_shape[d] == 1) {
                    origIdx[d] = 0;
                } else {
                    FatalError("Tensor u-broadcast shape err");
                }
            }
        }
        outTensor.locate(progIdx) = locate(origIdx);
        return;
    }
    for (size_t i = 0;i < outTensor.m_shape[x];i++) {
        progIdx[x] = i;
        unidirectionalBroadcastCopy(x + 1, progIdx, outTensor);
        progIdx[x] = 0;
    }
}


// Multidirectional broadcast
// https://github.com/onnx/onnx/blob/main/docs/Broadcasting.md
void Tensor::multidirectionalBroadcast(std::initializer_list<std::reference_wrapper<Tensor>> srcTensors) {
    // Determine dimension
    size_t destDim = 0;
    for (const auto& srcTensorRef : srcTensors) {
        destDim = std::max(destDim, srcTensorRef.get().m_shape.size());
    }

    // Prepend 1
    for (const auto& srcTensorRef : srcTensors) {
        auto& srcTensor = srcTensorRef.get();
        srcTensor.m_shape.insert(srcTensor.m_shape.begin(), destDim - srcTensor.m_shape.size(), 1);
    }

    // Determine shape
    Shape destShape(destDim, 0);
    for (size_t i = 0;i < destDim;i++) {
        for (const auto& srcTensorRef : srcTensors) {
            destShape[i] = std::max(destShape[i], srcTensorRef.get().m_shape[i]);
        }
    }

    // Apply u-broadcast
    for (const auto& srcTensorRef : srcTensors) {
        srcTensorRef.get().unidirectionalBroadcast(destShape);
    }
}
