# vec_send_scalar
add_executable(vec_send_scalar vec_send_scalar.cpp)
target_include_directories(vec_send_scalar PUBLIC ${PROJECT_SOURCE_DIR}/vec_core)
target_link_libraries(vec_send_scalar vec_core)


# mat_comm
add_executable(mat_comm mat_comm.cpp)
target_include_directories(mat_comm PUBLIC ${PROJECT_SOURCE_DIR}/mat_core)
target_link_libraries(mat_comm mat_core)
