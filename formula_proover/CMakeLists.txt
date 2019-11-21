cmake_minimum_required (VERSION 2.8.11)

add_subdirectory(../formula_lib formula_lib)

include_directories(${FORMULA_LIB_INCLUDE_DIR})

set(executable formula_proover)
add_executable(${executable} source.cpp)
target_link_libraries(${executable} formula_lib)

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
