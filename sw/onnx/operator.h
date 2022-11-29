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
        std::map<std::string, Shape>&);

    virtual void simulate(const ::onnx::NodeProto&,
        std::map<std::string, Tensor>&);
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


#endif
