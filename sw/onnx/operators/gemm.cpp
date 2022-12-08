#include "operators/gemm.h"
#include "logging.h"
#include "error.h"


// Get attributes from protobuf
void OperatorGemm::getAttributes(const ::onnx::NodeProto& node) {
    // Fetch transA and transB
    attr_transA = attr_transB = false;
    std::map<std::string, ::onnx::AttributeProto> attrs;
    for (const auto& attr : node.attribute()) {
        attrs[attr.name()] = attr;
    }
    if (attrs.count("transA") != 0) {
        const auto& attrProtoTransA = attrs.at("transA");
        if (attrProtoTransA.type() != ::onnx::AttributeProto::INT) {
            FatalError("Gemm transA not int type");
        }
        attr_transA = (bool)attrProtoTransA.i();
    }
    if (attrs.count("transB") != 0) {
        const auto& attrProtoTransB = attrs.at("transB");
        if (attrProtoTransB.type() != ::onnx::AttributeProto::INT) {
            FatalError("Gemm transB not int type");
        }
        attr_transB = (bool)attrProtoTransB.i();
    }

    // Fetch alpha and beta
    attr_alpha = attr_beta = 1.0f;
    if (attrs.count("alpha") != 0) {
        const auto& attrProtoAlpha = attrs.at("alpha");
        if (attrProtoAlpha.type() != ::onnx::AttributeProto::FLOAT) {
            FatalError("Gemm alpha not float type");
        }
        attr_alpha = (float)attrProtoAlpha.f();
    }
    if (attrs.count("beta") != 0) {
        const auto& attrProtoBeta = attrs.at("beta");
        if (attrProtoBeta.type() != ::onnx::AttributeProto::FLOAT) {
            FatalError("Gemm alpha not float type");
        }
        attr_beta = (float)attrProtoBeta.f();
    }
}


// InferShape for Gemm
void OperatorGemm::inferShape(const ::onnx::NodeProto& node,
        const std::map<std::string, Tensor>& stateInitializer,
        std::map<std::string, Tensor::Shape>& stateShape) {

    // Fetch A and B
    if (node.input_size() != 2 && node.input_size() != 3) {
        FatalError("Gemm input size " + std::to_string(node.input_size()));
    }
    Tensor::Shape shapeA, shapeB;
    if (stateShape.count(node.input(0)) == 0 ||
        stateShape.count(node.input(1)) == 0) {
        FatalError("Gemm cannot determine shape A/B " + node.input(0) + "/" + node.input(1));
    }
    shapeA = stateShape.at(node.input(0));
    shapeB = stateShape.at(node.input(1));
    if (shapeA.size() != 2 || shapeB.size() != 2) {
        FatalError("Gemm input dimension not 2D");
    }

    // Get transA/B
    getAttributes(node);

    // Do transA/B
    if (attr_transA) std::swap(shapeA[0], shapeA[1]);
    if (attr_transB) std::swap(shapeB[0], shapeB[1]);
    if (shapeA[1] != shapeB[0]) {
        FatalError("Gemm mat mult dim mismatch");
    }

    // Output shape
    Tensor::Shape cur_shape{
        shapeA[0], shapeB[1]
    };
    if (node.output_size() != 1) {
        FatalError("Gemm output size not 1");
    }
    stateShape[node.output(0)] = cur_shape;
}


// Simulate for Gemm
void OperatorGemm::simulate(const ::onnx::NodeProto& node,
        std::map<std::string, Tensor>& stateTensor) {

    // Fetch A and B
    if (node.input_size() != 2 && node.input_size() != 3) {
        FatalError("Gemm input size " + std::to_string(node.input_size()));
    }
    if (stateTensor.count(node.input(0)) == 0 ||
        stateTensor.count(node.input(1)) == 0) {
        FatalError("Gemm cannot determine shape A/B " + node.input(0) + "/" + node.input(1));
    }
    const Tensor& origTensorA = stateTensor.at(node.input(0)),
                  origTensorB = stateTensor.at(node.input(1));

    if (origTensorA.m_shape.size() != 2 || origTensorB.m_shape.size() != 2) {
        FatalError("Gemm input dimension not 2D");
    }

    // Fetch transA and transB
    getAttributes(node);
    Tensor* pTensorA = NULL, *pTensorB = NULL;

    if (!attr_transA) {
        pTensorA = new Tensor(origTensorA);
    } else {
        pTensorA = new Tensor(Tensor::Shape{
            origTensorA.m_shape[1],
            origTensorA.m_shape[0]
        });
        for (size_t i = 0;i < origTensorA.m_shape[1];i++) {
            for (size_t j = 0;j < origTensorA.m_shape[0];j++) {
                pTensorA->locate({i, j}) = origTensorA.locate({j, i});
            }
        }
    }
    if (!attr_transB) {
        pTensorB = new Tensor(origTensorB);
    } else {
        pTensorB = new Tensor(Tensor::Shape{
            origTensorB.m_shape[1],
            origTensorB.m_shape[0]
        });
        for (size_t i = 0;i < origTensorB.m_shape[1];i++) {
            for (size_t j = 0;j < origTensorB.m_shape[0];j++) {
                pTensorB->locate({i, j}) = origTensorB.locate({j, i});
            }
        }
    }
    // Do multiplication
    if (pTensorA->m_shape[1] != pTensorB->m_shape[0]) {
        FatalError("Gemm tensor dim mismatch");
    }
    size_t A = pTensorA->m_shape[0], B = pTensorA->m_shape[1], C = pTensorB->m_shape[1];
    Tensor tensorOut(Tensor::Shape{A, C});
    for (size_t j = 0;j < A;j++)
        for (size_t k = 0;k < C;k++) {
            float sum = 0;
            for (size_t i = 0;i < B;i++) {
                sum += pTensorA->m_value[j * B + i] * pTensorB->m_value[i * C + k];
            }
            tensorOut.m_value[j * C + k] = attr_alpha * sum;
        }

    if (node.input_size() == 3) {
        // Add matrix C
        Tensor tensorC = stateTensor.at(node.input(2));
        tensorC.unidirectionalBroadcast(tensorOut.m_shape);

        for (size_t i = 0;i < tensorOut.m_shape[0];i++)
            for (size_t j = 0;j < tensorOut.m_shape[1];j++)
                tensorOut.locate({i, j}) += attr_beta * tensorC.locate({i, j});
    }

    // Set output
    if (node.output_size() != 1) {
        FatalError("Gemm output size not 1");
    }
    stateTensor.emplace(node.output(0), tensorOut);

    delete pTensorA;
    delete pTensorB;
}

// Compile for gemm
void OperatorGemm::compile(const ::onnx::NodeProto& node,
        Orchestrator& orch,
        std::map<std::string, Orchestrator::MatrixHandle>& stateTensorHandles) {
    // Fetch A and B
    if (node.input_size() != 2 && node.input_size() != 3) {
        FatalError("Gemm input size " + std::to_string(node.input_size()));
    }
    if (stateTensorHandles.count(node.input(0)) == 0 ||
        stateTensorHandles.count(node.input(1)) == 0) {
        FatalError("Gemm cannot determine shape A/B " + node.input(0) + "/" + node.input(1));
    }
    auto handleA = stateTensorHandles.at(node.input(0)),
         handleB = stateTensorHandles.at(node.input(1));

    // Get attributes
    getAttributes(node);

    // Call orchestrator
    auto handleRes = orch.arithmeticMatMult(handleA, handleB);

    // Set output
    if (node.output_size() != 1) {
        FatalError("Gemm output size not 1");
    }
    stateTensorHandles[node.output(0)] = handleRes;
}
