cmake_minimum_required (VERSION 2.8.11)
MACRO(GET_CPP_H_RECURSIVE DIR_NAME EXEC_LIST) ##TODO-Add it as a module
      file(GLOB_RECURSE SOURCES "${DIR_NAME}/*.cpp" "${DIR_NAME}/*.h" "${DIR_NAME}/*.hpp")
      list(SORT SOURCES)
      SET(${EXEC_LIST} ${SOURCES})
ENDMACRO()

set(sources)
set(target_name formula_proover)

GET_CPP_H_RECURSIVE(${CMAKE_CURRENT_SOURCE_DIR} sources)
add_executable(${target_name} ${sources})
