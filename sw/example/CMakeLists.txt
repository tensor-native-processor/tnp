# mat_mult
add_executable(mat_mult mat_mult.cpp)
target_include_directories(mat_mult PUBLIC ${PROJECT_SOURCE_DIR}/mat_core)
target_link_libraries(mat_mult mat_core)

# mat_mult_flip
add_executable(mat_mult_flip mat_mult_flip.cpp)
target_include_directories(mat_mult_flip PUBLIC ${PROJECT_SOURCE_DIR}/mat_core)
target_link_libraries(mat_mult_flip mat_core)

# mat_mult_pipeline2
add_executable(mat_mult_pipeline2 mat_mult_pipeline2.cpp)
target_include_directories(mat_mult_pipeline2 PUBLIC ${PROJECT_SOURCE_DIR}/mat_core)
target_link_libraries(mat_mult_pipeline2 mat_core)

# mat_mult_pipeline4
add_executable(mat_mult_pipeline4 mat_mult_pipeline4.cpp)
target_include_directories(mat_mult_pipeline4 PUBLIC ${PROJECT_SOURCE_DIR}/mat_core)
target_link_libraries(mat_mult_pipeline4 mat_core)

# mat_asm
add_executable(mat_asm mat_asm.cpp)
target_include_directories(mat_asm PUBLIC ${PROJECT_SOURCE_DIR}/mat_core)
target_link_libraries(mat_asm mat_core)

# mat_sim
add_executable(mat_sim mat_sim.cpp)
target_include_directories(mat_sim PUBLIC ${PROJECT_SOURCE_DIR}/mat_core)
target_link_libraries(mat_sim mat_core)

# vec_add
add_executable(vec_add vec_add.cpp)
target_include_directories(vec_add PUBLIC ${PROJECT_SOURCE_DIR}/vec_core)
target_link_libraries(vec_add vec_core)

# vec_scale
add_executable(vec_scale vec_scale.cpp)
target_include_directories(vec_scale PUBLIC ${PROJECT_SOURCE_DIR}/vec_core)
target_link_libraries(vec_scale vec_core)

# vec_copy
add_executable(vec_copy vec_copy.cpp)
target_include_directories(vec_copy PUBLIC ${PROJECT_SOURCE_DIR}/vec_core)
target_link_libraries(vec_copy vec_core)

# vec_sim
add_executable(vec_sim vec_sim.cpp)
target_include_directories(vec_sim PUBLIC ${PROJECT_SOURCE_DIR}/vec_core)
target_link_libraries(vec_sim vec_core)

# vec_comm
add_executable(vec_comm vec_comm.cpp)
target_include_directories(vec_comm PUBLIC ${PROJECT_SOURCE_DIR}/vec_core)
target_link_libraries(vec_comm vec_core)



# vec_sim2
add_executable(vec_sim2 vec_sim2.cpp)
target_include_directories(vec_sim2 PUBLIC ${PROJECT_SOURCE_DIR}/vec_core)
target_link_libraries(vec_sim2 vec_core)
