# vec_send_scalar
add_executable(vec_send_scalar vec_send_scalar.cpp)
target_include_directories(vec_send_scalar PUBLIC ${PROJECT_SOURCE_DIR}/vec_core)
target_link_libraries(vec_send_scalar vec_core)
