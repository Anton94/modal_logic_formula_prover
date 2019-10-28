MACRO(GET_CPP_H_RECURSIVE DIR_NAME EXEC_LIST)
      file(GLOB_RECURSE SOURCES "${DIR_NAME}/*.cpp" "${DIR_NAME}/*.h" "${DIR_NAME}/*.hpp")
      list(SORT SOURCES)
      SET(${EXEC_LIST} ${SOURCES})
ENDMACRO()
