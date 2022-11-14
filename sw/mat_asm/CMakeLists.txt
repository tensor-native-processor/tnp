cmake_minimum_required(VERSION 3.0)
project(MatAsm)
set(CMAKE_CXX_STANDARD 17)

set(SRC mat_program.cpp mat_asm.cpp)

set(CMAKE_BUILD_TYPE RELEASE)
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall")

add_executable(matasm ${SRC})
