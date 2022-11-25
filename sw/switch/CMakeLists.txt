add_library(switch STATIC switch.cpp)

target_include_directories(switch PUBLIC ${PROJECT_SOURCE_DIR}/common)
target_link_libraries(switch common)
