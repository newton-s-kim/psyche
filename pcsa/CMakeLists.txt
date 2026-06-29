cmake_minimum_required(VERSION 3.12)

file(GLOB LAX_SOURCES ../psyche/*.c ../byte-code/src/*.c)
add_executable(pcsa ${LAX_SOURCES})
target_include_directories(pcsa PRIVATE . ../psyche ../byte-code/src)
set_target_properties(pcsa PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF)
target_compile_options(pcsa PRIVATE -Werror -Wall -Wextra -Wno-stringop-truncation)
#target_compile_definitions(pcsa PRIVATE $<$<CONFIG:Debug>:LAX_GC_DEBUG=1;LAX_DEBUG=1>)
target_link_libraries(pcsa readline m)
