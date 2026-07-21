cmake_minimum_required(VERSION 3.12)

project(raspindi C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

add_subdirectory(src)


add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )
