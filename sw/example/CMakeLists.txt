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

# mat_asm
add_executable(mat_asm mat_asm.cpp)
target_include_directories(mat_asm PUBLIC ${PROJECT_SOURCE_DIR}/mat_core)
target_link_libraries(mat_asm mat_core)
