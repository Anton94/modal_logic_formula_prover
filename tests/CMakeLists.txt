cmake_minimum_required (VERSION 2.8.11)

add_subdirectory(../formula_lib formula_lib)

include_directories(${FORMULA_LIB_INCLUDE_DIR})

add_executable(tests source.cpp satisfiability_tests.cpp variables_retrieving.cpp)
target_link_libraries(tests formula_lib)
