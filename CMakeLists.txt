cmake_minimum_required (VERSION 2.8.11)
project (ModalLogicFormulaProover)

set(CMAKE_CXX_STANDARD 14)
include_directories(${CMAKE_SOURCE_DIR}/library)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wconversion -pedantic -Wno-unknown-pragmas")
endif()

find_package(Boost REQUIRED)
if(Boost_FOUND)
	message("Found boost. Including header only libraries.")
	include_directories(${Boost_INCLUDE_DIRS})
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/modules)

add_subdirectory (parser)
add_subdirectory (formula_lib)
add_subdirectory (formula_proover)
add_subdirectory (rest_server)
add_subdirectory (tests)
add_subdirectory (client)

