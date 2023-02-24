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

if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    if(NOT CMAKE_TOOLCHAIN_FILE)
        message(FATAL_ERROR "CMAKE_TOOLCHAIN_FILE is not provided. Add cmake argument -DCMAKE_TOOLCHAIN_FILE=<path_to_your_emsdk_repo>/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")
    endif()

    if(NOT EMSCRIPTEN_INCLUDE_DIR)
        message(FATAL_ERROR "EMSCRIPTEN_INCLUDE_DIR variable is not set. Add cmake argument -DEMSCRIPTEN_INCLUDE_DIR=<path_to_your_emscript_repo>/upstream/emscripten/cache/sysroot/")
    else()
        message("Adding include directory for emscripten: " ${EMSCRIPTEN_INCLUDE_DIR})
        include_directories(${EMSCRIPTEN_INCLUDE_DIR})
    endif()

    # By default, exception catching is disabled in Emscripten but we are using them - https://emscripten.org/docs/porting/exceptions.html
    # This brings realitvely high overhead
    set(ENABLE_EXCEPTIONS_FLAG "-fexceptions") # In the future use fwasm-exceptions as it will be supported in all browsers
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ENABLE_EXCEPTIONS_FLAG}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ENABLE_EXCEPTIONS_FLAG}")
    add_link_options(${ENABLE_EXCEPTIONS_FLAG})
    add_compile_options(${ENABLE_EXCEPTIONS_FLAG})
    message("Adding compiler and linker option to enable exceptions in WASM: ${ENABLE_EXCEPTIONS_FLAG}")
endif()

add_subdirectory (parser_lib)
add_subdirectory (formula_lib)
add_subdirectory (formula_proover)

if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    message("Adding formula_proover_wasm project.")
    add_subdirectory (formula_proover_wasm)
endif()

add_subdirectory (performance_measure)
add_subdirectory (rest_server)
add_subdirectory (tests)
#add_subdirectory (client)
add_subdirectory (parser)

