cmake_minimum_required (VERSION 2.8.11)

include_directories(${PARSER_LIB_INCLUDE_DIR})
add_executable(parser main.cpp )
target_link_libraries(parser parser_lib)

#list(APPEND ECM_ENABLE_SANITIZERS "thread")
#list(APPEND ECM_ENABLE_SANITIZERS "memory") # memory is not allowed with address
list(APPEND ECM_ENABLE_SANITIZERS "address;undefined;leak")
include(ECMEnableSanitizers)
