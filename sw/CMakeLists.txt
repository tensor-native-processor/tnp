cmake_minimum_required(VERSION 3.0)
project(TensorNativeProcessor)

option(BUILD_ONNX "Build ONNX module" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE RELEASE)
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall")

add_subdirectory(common)
add_subdirectory(mat_core)
add_subdirectory(vec_core)
add_subdirectory(switch)
add_subdirectory(example)
add_subdirectory(simulation)
add_subdirectory(test)
add_subdirectory(compiler)

if(BUILD_ONNX)
    find_package(Protobuf REQUIRED)
    add_subdirectory(onnx)
endif(BUILD_ONNX)
