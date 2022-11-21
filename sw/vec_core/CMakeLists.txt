add_library(vec_core STATIC vec_program.cpp)

target_include_directories(vec_core PUBLIC ${PROJECT_SOURCE_DIR}/common)
target_link_libraries(vec_core common)
