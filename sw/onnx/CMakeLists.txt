# onnx
add_executable(onnx test.cpp model.cpp operator.cpp onnx.proto3.pb.cc operators/gemm.cpp operators/relu.cpp)

target_include_directories(onnx PUBLIC ${PROJECT_SOURCE_DIR}/onnx)
target_include_directories(onnx PUBLIC ${PROJECT_SOURCE_DIR}/common)

target_link_libraries(onnx protobuf)
target_link_libraries(onnx common)
