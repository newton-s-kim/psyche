cmake_minimum_required(VERSION 3.12)

add_library (dlmalloc malloc.c malloc.h)
target_compile_options(dlmalloc PRIVATE -fPIC -DUSE_DL_PREFIX)
