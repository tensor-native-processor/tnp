add_executable(mat_compiler mat_compiler.cpp)
target_include_directories(mat_compiler PUBLIC ${PROJECT_SOURCE_DIR}/mat_core)
target_link_libraries(mat_compiler mat_core)
