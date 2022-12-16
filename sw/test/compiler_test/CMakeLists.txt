# orch_init
add_executable(orch_init orch_init.cpp)
target_include_directories(orch_init PUBLIC ${PROJECT_SOURCE_DIR}/compiler)
target_link_libraries(orch_init compiler)


# orch_transpose
add_executable(orch_transpose orch_transpose.cpp)
target_include_directories(orch_transpose PUBLIC ${PROJECT_SOURCE_DIR}/compiler)
target_include_directories(orch_transpose PUBLIC ${PROJECT_SOURCE_DIR}/onnx)
target_link_libraries(orch_transpose compiler)
target_link_libraries(orch_transpose tensor)


# orch_transpose_self
add_executable(orch_transpose_self orch_transpose_self.cpp)
target_include_directories(orch_transpose_self PUBLIC ${PROJECT_SOURCE_DIR}/compiler)
target_include_directories(orch_transpose_self PUBLIC ${PROJECT_SOURCE_DIR}/onnx)
target_link_libraries(orch_transpose_self compiler)
target_link_libraries(orch_transpose_self tensor)


# orch_relu
add_executable(orch_relu orch_relu.cpp)
target_include_directories(orch_relu PUBLIC ${PROJECT_SOURCE_DIR}/compiler)
target_include_directories(orch_relu PUBLIC ${PROJECT_SOURCE_DIR}/onnx)
target_link_libraries(orch_relu compiler)
target_link_libraries(orch_relu tensor)


# orch_relu_transpose
add_executable(orch_relu_transpose orch_relu_transpose.cpp)
target_include_directories(orch_relu_transpose PUBLIC ${PROJECT_SOURCE_DIR}/compiler)
target_include_directories(orch_relu_transpose PUBLIC ${PROJECT_SOURCE_DIR}/onnx)
target_link_libraries(orch_relu_transpose compiler)
target_link_libraries(orch_relu_transpose tensor)


# orch_matmul
add_executable(orch_matmul orch_matmul.cpp)
target_include_directories(orch_matmul PUBLIC ${PROJECT_SOURCE_DIR}/compiler)
target_include_directories(orch_matmul PUBLIC ${PROJECT_SOURCE_DIR}/onnx)
target_link_libraries(orch_matmul compiler)
target_link_libraries(orch_matmul tensor)
