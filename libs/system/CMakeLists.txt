cmake_minimum_required(VERSION 3.12)

file(GLOB SYSTEM_SOURCES *.c)
add_library(scsystem SHARED ${SYSTEM_SOURCES})
target_include_directories(scsystem PRIVATE . ../../byte-code/src ../../nsl)
set_target_properties(scsystem PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF)
target_compile_options(scsystem PRIVATE -Werror -Wall -Wextra)
#target_compile_definitions(scsystem PRIVATE $<$<CONFIG:Debug>:LAX_GC_DEBUG=1;LAX_DEBUG=1>)
target_link_libraries(scsystem)
