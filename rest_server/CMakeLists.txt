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


set(executable rest_server)
add_executable(${executable} source.cpp microservice_controller.cpp microservice_controller.h task_result.cpp task_result.h task_info.cpp task_info.h command_runner.h command_runner.cpp)
target_link_libraries(${executable} formula_lib cpprestsdk::cpprest)

if(UNIX)
    target_link_libraries(${executable} ${Boost_SYSTEM_LIBRARY})
elseif(MSVC)
    target_compile_options(${executable} PRIVATE -bigobj)
endif()

if(${ENABLE_THREAD_SANITIZER})
    message("Enabling thread sanitizer for " ${executable} " app")
    list(APPEND ECM_ENABLE_SANITIZERS "thread")
endif()
if(${ENABLE_MEMORY_SANITIZER})
    message("Enabling memory sanitizer for " ${executable} " app")
    list(APPEND ECM_ENABLE_SANITIZERS "memory") # memory is not allowed with address
elseif(${ENABLE_ADDRESS_UB_LEAK_SANITIZERS})
    message("Enabling address undefined leak sanitizer for " ${executable} " app")
    list(APPEND ECM_ENABLE_SANITIZERS "address;undefined;leak")
endif()

include(ECMEnableSanitizers)

add_custom_command(
  TARGET ${executable} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/simple_client
          ${CMAKE_BINARY_DIR}/client/dist)

# Replace the api_wrapper with the one which uses server_api.js
add_custom_command(
  TARGET ${executable} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
          ${CMAKE_BINARY_DIR}/client/dist/rest_server/api_wrapper.js
          ${CMAKE_BINARY_DIR}/client/dist/api_wrapper.js)
