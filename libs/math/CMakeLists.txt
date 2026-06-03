cmake_minimum_required(VERSION 3.12)

file(GLOB MATH_SOURCES *.cpp)
find_package(Armadillo REQUIRED)
add_library(scmath SHARED ${MATH_SOURCES})
target_include_directories(scmath PRIVATE . ../../byte-code/src ../../nsl)
set_target_properties(scmath PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF)
target_compile_options(scmath PRIVATE -Werror -Wall -Wextra)
#target_compile_definitions(scmath PRIVATE $<$<CONFIG:Debug>:LAX_GC_DEBUG=1;LAX_DEBUG=1>)
target_link_libraries(scmath armadillo)
