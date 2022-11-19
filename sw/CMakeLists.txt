cmake_minimum_required(VERSION 3.0)
project(TensorNativeProcessor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE RELEASE)
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall")

add_subdirectory(common)
add_subdirectory(mat_core)
add_subdirectory(example)
