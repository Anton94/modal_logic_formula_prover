cmake_minimum_required (VERSION 2.8.11)

MACRO(GET_CPP_H_RECURSIVE DIR_NAME EXEC_LIST) ##TODO-Add it as a module
      file(GLOB_RECURSE SOURCES "${DIR_NAME}/*.cpp" "${DIR_NAME}/*.h" "${DIR_NAME}/*.hpp")
      list(SORT SOURCES)
      SET(${EXEC_LIST} ${SOURCES})
ENDMACRO()

set(sources)

GET_CPP_H_RECURSIVE(${CMAKE_CURRENT_SOURCE_DIR} sources)

set(FORMULA_LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(FORMULA_LIB_INCLUDE_DIR ${FORMULA_LIB_INCLUDE_DIR} PARENT_SCOPE)

set(FORMULA_LIB_PRIVATE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/private)
set(FORMULA_LIB_PRIVATE_DIR ${FORMULA_LIB_PRIVATE_DIR} PARENT_SCOPE)

if(TARGET formula_lib)
    message("formula_lib is already defined")
else()
    include_directories(${FORMULA_LIB_INCLUDE_DIR})
    include_directories(${PARSER_LIB_INCLUDE_DIR})

    message("Creating formula_lib")
    set(LIB_SRCS ${sources})
    add_library(formula_lib STATIC ${LIB_SRCS})

    message("Linking formula_lib with parser_lib")
    target_link_libraries(formula_lib parser_lib)
endif()
