#ifndef __OPERATOR_H__
#define __OPERATOR_H__

#include "model.h"

#include <map>
#include <string>
#include <utility>


// Generic operator
class Operator {
public:
    // Destructor
    virtual ~Operator() {}

    // Operations
    virtual void inferShape(const ::onnx::NodeProto&,
        const std::map<std::string, Tensor>&,
        std::map<std::string, Shape>&) const;

    virtual void simulate(const ::onnx::NodeProto&,
        std::map<std::string, Tensor>&) const;
};

// Dispatch operators
class OperatorDispatch {
public:
    static Operator* createOperator(const std::string&);

private:
    static const std::map<std::string, Operator*(*)()> g_operatorDirectory;

    template<class T>
    static Operator* createOperatorImpl();
};


// Gemm
class OperatorGemm: public Operator {
    void inferShape(const ::onnx::NodeProto&,
        const std::map<std::string, Tensor>&,
        std::map<std::string, Shape>&) const override;

    void simulate(const ::onnx::NodeProto&,
        std::map<std::string, Tensor>&) const override;
};


// Relu
class OperatorRelu: public Operator {
    void inferShape(const ::onnx::NodeProto&,
        const std::map<std::string, Tensor>&,
        std::map<std::string, Shape>&) const override;

    void simulate(const ::onnx::NodeProto&,
        std::map<std::string, Tensor>&) const override;
};

#endif
