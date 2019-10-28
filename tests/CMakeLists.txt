cmake_minimum_required (VERSION 2.8.11)

add_subdirectory(../formula_lib formula_lib)

include_directories(${FORMULA_LIB_INCLUDE_DIR})
include_directories(${PARSER_LIB_INCLUDE_DIR})

message ("Including '${FORMULA_LIB_PRIVATE_DIR}' in order to test it")
include_directories(${FORMULA_LIB_PRIVATE_DIR})

include(ECMUtilities)
set(sources)
GET_CPP_H_RECURSIVE(${CMAKE_CURRENT_SOURCE_DIR} sources)

add_executable(tests ${sources})
target_link_libraries(tests formula_lib)
