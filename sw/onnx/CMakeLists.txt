# onnx
add_executable(onnx main.cpp onnx.proto3.pb.cc)
target_link_libraries(onnx protobuf)
