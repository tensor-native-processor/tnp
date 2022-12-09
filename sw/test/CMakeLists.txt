add_subdirectory(hwtest)

if(HAS_PROTOBUF)
add_subdirectory(compiler_test)
endif(HAS_PROTOBUF)
