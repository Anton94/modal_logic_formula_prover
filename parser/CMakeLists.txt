cmake_minimum_required (VERSION 2.8.11)

include_directories(${PARSER_LIB_INCLUDE_DIR})
set(executable parser)
add_executable(${executable} main.cpp )
target_link_libraries(${executable} parser_lib)

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
