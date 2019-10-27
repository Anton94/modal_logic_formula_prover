cmake_minimum_required (VERSION 2.8.11)
project (ModalLogicFormulaProover)

set(CMAKE_CXX_STANDARD 14)
include_directories(${CMAKE_SOURCE_DIR}/library)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wconversion -pedantic -Wno-unknown-pragmas")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/modules)

add_subdirectory (parser_lib)
add_subdirectory (formula_lib)
add_subdirectory (formula_proover)
add_subdirectory (rest_server)
add_subdirectory (tests)
#add_subdirectory (client)
add_subdirectory (parser)

if(${ENABLE_THREAD_SANITIZER})
    list(APPEND ECM_ENABLE_SANITIZERS "thread")
endif()
if(${ENABLE_MEMORY_SANITIZER})
    list(APPEND ECM_ENABLE_SANITIZERS "memory") # memory is not allowed with address
elseif(${ENABLE_ADDRESS_UB_LEAK_SANITIZERS})
    list(APPEND ECM_ENABLE_SANITIZERS "address;undefined;leak")
endif()

include(ECMEnableSanitizers)
