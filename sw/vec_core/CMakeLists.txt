add_library(vec_core STATIC vec_program.cpp vec_sim.cpp)

target_include_directories(vec_core PUBLIC ${PROJECT_SOURCE_DIR}/common)
target_include_directories(vec_core PUBLIC ${PROJECT_SOURCE_DIR}/switch)
target_link_libraries(vec_core common)
target_link_libraries(vec_core switch)
