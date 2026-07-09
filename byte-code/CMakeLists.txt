cmake_minimum_required(VERSION 3.12)

file(GLOB PSYCHE_SOURCES src/*.c)
add_library(pcvm SHARED ${PSYCHE_SOURCES})
target_include_directories(pcvm PRIVATE src ../foss)
set_target_properties(pcvm PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF)
target_compile_options(pcvm PRIVATE -Werror -Wall -Wextra)
#target_compile_definitions(pcvm PRIVATE $<$<CONFIG:Debug>:LAX_GC_DEBUG=1;LAX_DEBUG=1>)
target_link_libraries(pcvm dlmalloc m)
