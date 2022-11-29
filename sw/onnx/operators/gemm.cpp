#include "operators/gemm.h"
#include "logging.h"
#include "error.h"

// InferShape for Gemm
void OperatorGemm::inferShape(const ::onnx::NodeProto& node,
        const std::map<std::string, Tensor>& state_initializer,
        std::map<std::string, Shape>& state_shape) const {

    // Fetch A and B
    if (node.input_size() != 2 && node.input_size() != 3) {
        FatalError("Gemm input size " + std::to_string(node.input_size()));
    }
    Shape shapeA, shapeB;
    if (state_shape.count(node.input(0)) == 0 ||
        state_shape.count(node.input(1)) == 0) {
        FatalError("Gemm cannot determine shape A/B " + node.input(0) + "/" + node.input(1));
    }
    shapeA = state_shape.at(node.input(0));
    shapeB = state_shape.at(node.input(1));
    if (shapeA.size() != 2 || shapeB.size() != 2) {
    std::cerr << shapeA.size() << ", " << shapeB.size() << std::endl;
        FatalError("Gemm input dimension not 2D");
    }

    // Fetch transA and transB
    bool transA = false, transB = false;
    std::map<std::string, ::onnx::AttributeProto> attrs;
    for (const auto& attr : node.attribute()) {
        attrs[attr.name()] = attr;
    }
    if (attrs.count("transA") != 0) {
        const auto& attrTransA = attrs.at("transA");
        if (attrTransA.type() != ::onnx::AttributeProto::INT) {
            FatalError("Gemm transA not int type");
        }
        transA = (bool)attrTransA.i();
    }
    if (attrs.count("transB") != 0) {
        const auto& attrTransB = attrs.at("transB");
        if (attrTransB.type() != ::onnx::AttributeProto::INT) {
            FatalError("Gemm transB not int type");
        }
        transB = (bool)attrTransB.i();
    }
    // Do transA/B
    if (transA) std::swap(shapeA[0], shapeA[1]);
    if (transB) std::swap(shapeB[0], shapeB[1]);
    if (shapeA[1] != shapeB[0]) {
        FatalError("Gemm mat mult dim mismatch");
    }

    // Output shape
    Shape cur_shape{
        shapeA[0], shapeB[1]
    };
    if (node.output_size() != 1) {
        FatalError("Gemm output size not 1");
    }
    state_shape[node.output(0)] = cur_shape;
}


// Simulate for Gemm
void OperatorGemm::simulate(const ::onnx::NodeProto& node,
        std::map<std::string, Tensor>& state_tensor) const {

    LogWarning("Simulate " + node.name());
}
