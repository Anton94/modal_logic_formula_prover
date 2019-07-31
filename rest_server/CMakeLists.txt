cmake_minimum_required(VERSION 3.7)

add_subdirectory(../formula_lib formula_lib)

include_directories(${FORMULA_LIB_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(cpprestsdk NAMES cpprestsdk cpprest)

if (${cpprestsdk_FOUND})
    message(STATUS "Found cpprestsdk.")
    add_executable(rest_server source.cpp microservice_controller.cpp microservice_controller.h)

    find_package(Boost REQUIRED COMPONENTS filesystem)
    target_link_libraries(rest_server formula_lib cpprestsdk::cpprest Boost::filesystem)
else (${cpprestsdk_FOUND})
    message(STATUS "Could not locate cpprestsdk. Skipping rest_server project!")
endif (${cpprestsdk_FOUND})
