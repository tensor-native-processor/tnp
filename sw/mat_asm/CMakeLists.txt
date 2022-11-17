add_executable(matasm mat_program.cpp mat_asm.cpp)

target_include_directories(matasm PUBLIC ${PROJECT_SOURCE_DIR}/common)
target_link_libraries(matasm common)
