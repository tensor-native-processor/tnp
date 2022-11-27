#include "model.h"
#include "logging.h"

ONNXModel::ONNXModel(const std::string& filename) {
    LogInfo("Loading model " + filename);
}
