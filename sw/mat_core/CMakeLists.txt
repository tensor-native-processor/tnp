add_library(mat_core STATIC mat_program.cpp mat_sim.cpp)

target_include_directories(mat_core PUBLIC ${PROJECT_SOURCE_DIR}/common)
target_link_libraries(mat_core common)
