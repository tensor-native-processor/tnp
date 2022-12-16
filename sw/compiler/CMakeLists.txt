add_executable(mat_compiler mat_compiler.cpp)
target_include_directories(mat_compiler PUBLIC ${PROJECT_SOURCE_DIR}/mat_core)
target_link_libraries(mat_compiler mat_core)
target_include_directories(mat_compiler PUBLIC ${PROJECT_SOURCE_DIR}/vec_core)
target_link_libraries(mat_compiler vec_core)


# Compiler library
add_library(compiler orchestration.cpp relu.cpp transpose.cpp simulation.cpp)
target_include_directories(compiler PUBLIC ${PROJECT_SOURCE_DIR}/compiler)
target_include_directories(compiler PUBLIC ${PROJECT_SOURCE_DIR}/common)
target_include_directories(compiler PUBLIC ${PROJECT_SOURCE_DIR}/mat_core)
target_include_directories(compiler PUBLIC ${PROJECT_SOURCE_DIR}/vec_core)
target_link_libraries(compiler common)
target_link_libraries(compiler mat_core)
target_link_libraries(compiler vec_core)
target_link_libraries(compiler switch)
