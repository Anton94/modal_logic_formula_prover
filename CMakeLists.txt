cmake_minimum_required (VERSION 2.8.11)
project (ModalLogicFormulaProover)

set(CMAKE_CXX_STANDARD 14)
include_directories(${CMAKE_SOURCE_DIR}/library)

if(NOT CMAKE_BUILD_TYPE)
    set(default_build_type "Debug")
    message("No build type was specified. Fallback to default - " ${default_build_type})
    set(CMAKE_BUILD_TYPE ${default_build_type})
endif()

message("Build type: " ${CMAKE_BUILD_TYPE})

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wconversion -pedantic -Wno-unknown-pragmas")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/modules)

add_subdirectory (parser_lib)
add_subdirectory (formula_lib)
add_subdirectory (formula_proover)
#add_subdirectory (formula_proover_wasm)
add_subdirectory (performance_measure)
add_subdirectory (rest_server)
add_subdirectory (tests)
#add_subdirectory (client)
add_subdirectory (parser)

