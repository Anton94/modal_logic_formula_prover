cmake_minimum_required (VERSION 2.8.11)

add_subdirectory(../formula_lib formula_lib)

include_directories(${FORMULA_LIB_INCLUDE_DIR})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s EXPORTED_FUNCTIONS=['_malloc','_free','_main'] -s EXTRA_EXPORTED_RUNTIME_METHODS=['cwrap']")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ALLOW_MEMORY_GROWTH=1") # Increase th heap memory limit

set(executable formula_proover_wasm)
add_executable(${executable} source.cpp)

target_link_libraries(${executable} formula_lib)

if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")

    get_target_property(MAIN_CFLAGS ${executable} COMPILE_OPTIONS)
    message("-- Target " ${executable} " compiler flags are: ${MAIN_CFLAGS}")

    get_target_property(MAIN_CFLAGS ${executable} LINK_OPTIONS)
    message("-- Target " ${executable} " link flags are: ${MAIN_CFLAGS}")

    target_compile_options(${executable} PUBLIC -s WASM=1) # make a separate .wasm file instead of adding the wasm to .js produced file.

    # target_link_options(${executable} PUBLIC -s EXTRA_EXPORTED_RUNTIME_METHODS=['cwrap']) # The exported functions to be visible in JS
    # target_link_options(${executable} PUBLIC -s EXPORTED_FUNCTIONS=["_malloc"])

    #target_compile_options(${executable} PUBLIC -s ALLOW_MEMORY_GROWTH=1) # Allow bigger memory allocation

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
        COMMAND ${CMAKE_COMMAND} -E copy # for directory try copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/index.html
                ${CMAKE_CURRENT_BINARY_DIR}/index.html)
