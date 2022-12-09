# mat_sim
add_executable(mat_sim mat_sim.cpp)
target_include_directories(mat_sim PUBLIC ${PROJECT_SOURCE_DIR}/mat_core)
target_link_libraries(mat_sim mat_core)


# vec_sim
add_executable(vec_sim vec_sim.cpp)
target_include_directories(vec_sim PUBLIC ${PROJECT_SOURCE_DIR}/vec_core)
target_link_libraries(vec_sim vec_core)


# vec_sim2
add_executable(vec_sim2 vec_sim2.cpp)
target_include_directories(vec_sim2 PUBLIC ${PROJECT_SOURCE_DIR}/vec_core)
target_link_libraries(vec_sim2 vec_core)


# mat_sim2
add_executable(mat_sim2 mat_sim2.cpp)
target_include_directories(mat_sim2 PUBLIC ${PROJECT_SOURCE_DIR}/mat_core)
target_link_libraries(mat_sim2 mat_core)


# proc_u-16m4-16v4_sim
add_executable(proc_u-16m4-16v4_sim proc_u-16m4-16v4_sim.cpp)
target_include_directories(proc_u-16m4-16v4_sim PUBLIC ${PROJECT_SOURCE_DIR}/mat_core)
target_include_directories(proc_u-16m4-16v4_sim PUBLIC ${PROJECT_SOURCE_DIR}/vec_core)
target_link_libraries(proc_u-16m4-16v4_sim mat_core)
target_link_libraries(proc_u-16m4-16v4_sim vec_core)


# mat_disas
add_executable(mat_disas mat_disas.cpp)
target_include_directories(mat_disas PUBLIC ${PROJECT_SOURCE_DIR}/mat_core)
target_include_directories(mat_disas PUBLIC ${PROJECT_SOURCE_DIR}/common)
target_link_libraries(mat_disas mat_core)
target_link_libraries(mat_disas common)


# vec_disas
add_executable(vec_disas vec_disas.cpp)
target_include_directories(vec_disas PUBLIC ${PROJECT_SOURCE_DIR}/vec_core)
target_include_directories(vec_disas PUBLIC ${PROJECT_SOURCE_DIR}/common)
target_link_libraries(vec_disas vec_core)
target_link_libraries(vec_disas common)
