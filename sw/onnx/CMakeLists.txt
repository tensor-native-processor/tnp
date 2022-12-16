# protobuf
include(FindProtobuf)
find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRC PROTO_HDR onnx.proto)

add_library(onnx_proto ${PROTO_SRC})
target_include_directories(onnx_proto PUBLIC ${PROTOBUF_INCLUDE_DIRS})
target_link_libraries(onnx_proto ${PROTOBUF_LIBRARIES})


# tensor
add_library(tensor tensor.cpp)
target_include_directories(tensor PUBLIC ${PROTOBUF_INCLUDE_DIRS})
target_include_directories(tensor PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(tensor PUBLIC ${PROJECT_SOURCE_DIR}/compiler)
target_include_directories(tensor PUBLIC ${PROJECT_SOURCE_DIR}/common)
target_link_libraries(tensor onnx_proto)
target_link_libraries(tensor compiler)
target_link_libraries(tensor common)

# onnx
add_executable(onnx test.cpp model.cpp operator.cpp operators/gemm.cpp operators/relu.cpp)

target_include_directories(onnx PUBLIC ${PROTOBUF_INCLUDE_DIRS})
target_include_directories(onnx PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(onnx PUBLIC ${PROJECT_SOURCE_DIR}/onnx)
target_include_directories(onnx PUBLIC ${PROJECT_SOURCE_DIR}/common)
target_include_directories(onnx PUBLIC ${PROJECT_SOURCE_DIR}/compiler)

target_link_libraries(onnx onnx_proto)
target_link_libraries(onnx common)
target_link_libraries(onnx compiler)
target_link_libraries(onnx tensor)
