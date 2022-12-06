# orch_init
add_executable(orch_init orch_init.cpp)
target_include_directories(orch_init PUBLIC ${PROJECT_SOURCE_DIR}/compiler)
target_link_libraries(orch_init compiler)
