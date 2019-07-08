cmake_minimum_required (VERSION 2.8.11)

add_subdirectory(../formula_lib formula_lib)

include_directories(${FORMULA_LIB_INCLUDE_DIR})

add_executable(formula_proover source.cpp)
target_link_libraries(formula_proover formula_lib)

FIND_PACKAGE( Boost 1.40 REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

TARGET_LINK_LIBRARIES(formula_proover LINK_PUBLIC ${Boost_LIBRARIES} )
