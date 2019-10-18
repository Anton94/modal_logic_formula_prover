cmake_minimum_required (VERSION 2.8.11)

add_subdirectory(../formula_lib formula_lib)

include_directories(${FORMULA_LIB_INCLUDE_DIR})

message ("Including '${FORMULA_LIB_PRIVATE_DIR}' in order to test it")
include_directories(${FORMULA_LIB_PRIVATE_DIR})

add_executable(tests source.cpp satisfiability_tests.cpp variables_retrieving_formula_tests.cpp variables_retrieving_term_tests.cpp variables_evaluations_block_tests.cpp variables_evaluations_block_stack_tests.cpp contact_commutativity_tests.cpp term_evaluation_subset_variables_tests.cpp common_tests.cpp system_of_inequalities_tests.cpp)
target_link_libraries(tests formula_lib)

#list(APPEND ECM_ENABLE_SANITIZERS "thread")
#list(APPEND ECM_ENABLE_SANITIZERS "memory") # memory is not allowed with address
list(APPEND ECM_ENABLE_SANITIZERS "address;undefined;leak")
include(ECMEnableSanitizers)
