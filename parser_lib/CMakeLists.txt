cmake_minimum_required (VERSION 3.5)

find_package(BISON)
find_package(FLEX)

set(generated_parser_cpp ${CMAKE_CURRENT_SOURCE_DIR}/generated/parser.cpp)
set(generated_parser_hpp ${CMAKE_CURRENT_SOURCE_DIR}/generated/parser.hpp)
set(generated_lexer_cpp ${CMAKE_CURRENT_SOURCE_DIR}/generated/lexer.cpp)
set(generated_lexer_hpp ${CMAKE_CURRENT_SOURCE_DIR}/generated/lexer.hpp)

set(pregenerated_msg "[PARSER] WARNING: Will use the pregenerated lexer and parser files! lexer.l and parser.y will not be processed and any changes on them are not going to change the files in the 'generated' subfolder!")

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message("[PARSER] WARNING: Flex and bison might not work properly with other than gcc compiler (therefore the parser, too)!")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/workaround_non_gcc_headers)
endif()

if (NOT BISON_FOUND)
    message("[PARSER] ERROR: Unable to find BISON.")
    message(${pregenerated_msg})
elseif (NOT FLEX_FOUND)
    message("[PARSER] ERROR: Unable to find FLEX.")
    message(${pregenerated_msg})
else()
    message("[PARSER] Found Bison: " ${BISON_VERSION})
    message("[PARSER] Found Flex: " ${FLEX_VERSION})
    if(BISON_VERSION VERSION_LESS "3.4")
        message("[PARSER] Bison version is less than 3.4 and not tested for compatability. If you insist, you can remove the check and try it.")
        message(${pregenerated_msg})
    elseif(FLEX_VERSION VERSION_LESS "2.6")
        message("[PARSER] Flex version is less than 2.6 and not tested for compatability. If you insist, you can remove the check and try it.")
        message(${pregenerated_msg})
    else()
        BISON_TARGET(Parser parser.y ${generated_parser_cpp})
        FLEX_TARGET(Scanner lexer.l ${generated_lexer_cpp} DEFINES_FILE "${generated_lexer_hpp}")
        ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)

        if((NOT ${generated_parser_cpp} STREQUAL ${BISON_Parser_OUTPUT_SOURCE}) OR
           (NOT ${generated_parser_hpp} STREQUAL ${BISON_Parser_OUTPUT_HEADER}) OR
           (NOT ${generated_lexer_cpp} STREQUAL ${FLEX_Scanner_OUTPUTS}) OR
           (NOT ${generated_lexer_hpp} STREQUAL ${FLEX_Scanner_OUTPUT_HEADER}))
            message("[PARSER] ERROR: For some reason the generated output files of bison/flex are not the same as the generated once. You can swap them by hand or fix the issue.")
            message("[PARSER] Expected generated files: " ${generated_parser_cpp} ${generated_parser_hpp} ${generated_lexer_cpp} ${generated_lexer_hpp})
            message("[PARSER] Bison generated files: " ${BISON_Parser_OUTPUTS})
            message("[PARSER] Flex generated files: " ${FLEX_Scanner_OUTPUTS})
        endif()
	endif()
endif()

# Create a static library in order to be used from other projects
message("Creating parser_lib")

set(PARSER_LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(PARSER_LIB_INCLUDE_DIR ${PARSER_LIB_INCLUDE_DIR} PARENT_SCOPE)

set(LIB_SRCS visitor.cpp ast.cpp parser_API.h parser_API.cpp visitor.h ast.h ${generated_parser_cpp} ${generated_lexer_cpp} ${generated_parser_hpp} ${generated_lexer_hpp})
add_library(parser_lib STATIC ${LIB_SRCS})
