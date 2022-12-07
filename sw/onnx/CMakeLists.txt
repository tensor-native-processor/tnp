# onnx
find_package(Protobuf REQUIRED)

add_executable(onnx test.cpp model.cpp tensor.cpp operator.cpp onnx.proto3.pb.cc operators/gemm.cpp operators/relu.cpp)

target_include_directories(onnx PUBLIC ${PROTOBUF_INCLUDE_DIRS})
target_include_directories(onnx PUBLIC ${PROJECT_SOURCE_DIR}/onnx)
target_include_directories(onnx PUBLIC ${PROJECT_SOURCE_DIR}/common)
target_include_directories(onnx PUBLIC ${PROJECT_SOURCE_DIR}/compiler)

target_link_libraries(onnx ${PROTOBUF_LIBRARIES})
target_link_libraries(onnx common)
target_link_libraries(onnx compiler)
