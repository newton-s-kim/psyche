cmake_minimum_required(VERSION 3.12)

project(sciche)

# Option to enable gprof profiling (OFF by default)
option(ENABLE_GPROF "Enable compiling with flags set for profiling with gprof." OFF)

if(ENABLE_GPROF)
    # Add the -pg flag for both compilation and linking
    # Use target_compile_options and target_link_libraries for modern CMake
    # Or set properties that propagate to subdirectories
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -pg")
endif()

add_subdirectory(byte-code)
add_subdirectory(libs/math)
add_subdirectory(psyche)
