cmake_minimum_required (VERSION 2.8.11)
project (ModalLogicFormulaProover)

set(CMAKE_CXX_STANDARD 14)
include_directories(${CMAKE_SOURCE_DIR}/library)

add_subdirectory (formula_lib)
add_subdirectory (formula_proover)
add_subdirectory (tests)
