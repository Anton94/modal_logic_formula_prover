cmake_minimum_required (VERSION 2.8.11)

find_package(BISON)
find_package(FLEX)

set(generated_parser_cpp ${CMAKE_CURRENT_SOURCE_DIR}/generated/parser.cpp)
set(generated_parser_hpp ${CMAKE_CURRENT_SOURCE_DIR}/generated/parser.hpp)
set(generated_lexer_cpp ${CMAKE_CURRENT_SOURCE_DIR}/generated/lexer.cpp)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message("ERROR: Flex and bison might not work properly with other than gcc compiler. Will use the pregenerated lexer and parser!")
elseif (NOT BISON_FOUND)
    message("ERROR: Unable to find BISON. Will use the pregenerated lexer and parser!")
elseif (NOT FLEX_FOUND)
    message("ERROR: Unable to find FLEX. Will use the pregenerated lexer and parser!")
else()
    BISON_TARGET(Parser parser.y ${generated_parser_cpp})
    FLEX_TARGET(Scanner lexer.l ${generated_lexer_cpp})
    ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})

    if((NOT ${generated_parser_cpp} STREQUAL ${BISON_Parser_OUTPUT_SOURCE}) OR
       (NOT ${generated_parser_hpp} STREQUAL ${BISON_Parser_OUTPUT_HEADER}) OR
       (NOT ${generated_lexer_cpp} STREQUAL ${FLEX_Scanner_OUTPUTS}))
        message("ERROR: For some reason the generated output files of bison/flex are not the same as the generated once. You can swap them by hand or fix the issue.")
        message("Expected generated files: " ${generated_parser_cpp} ${generated_parser_hpp} ${generated_lexer_cpp})
        message("Bison generated files: " ${BISON_Parser_OUTPUTS})
        message("Flex generated files: " ${FLEX_Scanner_OUTPUTS})
    endif()
endif()


add_executable(parser main.cpp visitor.cpp ast.cpp parser_API.h visitor.h ast.h ${generated_parser_cpp} ${generated_parser_hpp} ${generated_lexer_cpp})

#list(APPEND ECM_ENABLE_SANITIZERS "thread")
#list(APPEND ECM_ENABLE_SANITIZERS "memory") # memory is not allowed with address
list(APPEND ECM_ENABLE_SANITIZERS "address;undefined;leak")
include(ECMEnableSanitizers)
