add_executable(mat_compiler mat_compiler.cpp)
target_include_directories(mat_compiler PUBLIC ${PROJECT_SOURCE_DIR}/mat_core)
target_link_libraries(mat_compiler mat_core)
target_include_directories(mat_compiler PUBLIC ${PROJECT_SOURCE_DIR}/vec_core)
target_link_libraries(mat_compiler vec_core)


# Orchestration
add_library(orchestration orchestration.cpp)
target_include_directories(orchestration PUBLIC ${PROJECT_SOURCE_DIR}/mat_core)
target_include_directories(orchestration PUBLIC ${PROJECT_SOURCE_DIR}/vec_core)
target_link_libraries(orchestration mat_core)
target_link_libraries(orchestration vec_core)
