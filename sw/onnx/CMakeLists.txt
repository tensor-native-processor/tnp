# onnx
add_executable(onnx main.cpp onnx.proto3.pb.cc)

target_include_directories(onnx PUBLIC ${PROJECT_SOURCE_DIR}/common)

target_link_libraries(onnx protobuf)
target_link_libraries(onnx common)
