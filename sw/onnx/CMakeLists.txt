find_package(Protobuf REQUIRED)


# tensor
add_library(tensor tensor.cpp)
target_include_directories(tensor PUBLIC ${PROJECT_SOURCE_DIR}/compiler)
target_include_directories(tensor PUBLIC ${PROJECT_SOURCE_DIR}/common)
target_link_libraries(tensor ${PROTOBUF_LIBRARIES})
target_link_libraries(tensor compiler)
target_link_libraries(tensor common)

# onnx
add_executable(onnx test.cpp model.cpp operator.cpp onnx.proto3.pb.cc operators/gemm.cpp operators/relu.cpp)

target_include_directories(onnx PUBLIC ${PROTOBUF_INCLUDE_DIRS})
target_include_directories(onnx PUBLIC ${PROJECT_SOURCE_DIR}/onnx)
target_include_directories(onnx PUBLIC ${PROJECT_SOURCE_DIR}/common)
target_include_directories(onnx PUBLIC ${PROJECT_SOURCE_DIR}/compiler)

target_link_libraries(onnx ${PROTOBUF_LIBRARIES})
target_link_libraries(onnx common)
target_link_libraries(onnx compiler)
target_link_libraries(onnx tensor)
