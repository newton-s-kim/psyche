cmake_minimum_required(VERSION 3.12)

file(GLOB LAX_SOURCES *.c)
add_executable(psyche ${LAX_SOURCES})
target_include_directories(psyche PRIVATE . ../byte-code/src)
set_target_properties(psyche PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF)
target_compile_options(psyche PRIVATE -Werror -Wall -Wextra)
#target_compile_definitions(psyche PRIVATE $<$<CONFIG:Debug>:LAX_GC_DEBUG=1;LAX_DEBUG=1>)
target_link_libraries(psyche pcvm)
