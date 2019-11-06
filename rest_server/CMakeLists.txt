cmake_minimum_required(VERSION 3.7)

add_subdirectory(../formula_lib formula_lib)

include_directories(${FORMULA_LIB_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(cpprestsdk NAMES cpprestsdk cpprest)

if (NOT ${cpprestsdk_FOUND})
    message("[CPPRESTSDK] Could not locate cpprestsdk. Skipping rest_server project!")
    return()
endif()
message("[CPPRESTSDK] Found cpprestsdk.")

if(UNIX)
    find_package(Boost COMPONENTS system)
    if(NOT Boost_FOUND)
        message("[CPPRESTSDK] Could not locate boost, but for LINUX it's required ;/. Skipping rest_server project!")
        return()
    endif()
    message("[CPPRESTSDK] (Unix) Found boost.")
endif()


add_executable(rest_server source.cpp microservice_controller.cpp microservice_controller.h task_result.cpp task_result.h task_info.cpp task_info.h)
target_link_libraries(rest_server formula_lib cpprestsdk::cpprest)

if(UNIX)
    target_link_libraries(rest_server ${Boost_SYSTEM_LIBRARY})
elseif(MSVC)
    target_compile_options(rest_server PRIVATE -bigobj)
endif()
