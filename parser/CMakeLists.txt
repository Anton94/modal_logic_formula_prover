cmake_minimum_required (VERSION 2.8.11)

include_directories(${PARSER_LIB_INCLUDE_DIR})
add_executable(parser main.cpp )
target_link_libraries(parser parser_lib)