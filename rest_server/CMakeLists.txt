cmake_minimum_required(VERSION 3.7)

add_subdirectory(../formula_lib formula_lib)

include_directories(${FORMULA_LIB_INCLUDE_DIR})

find_package(cpprestsdk REQUIRED)

add_executable(rest_server source.cpp)
target_link_libraries(rest_server formula_lib cpprestsdk::cpprest)
