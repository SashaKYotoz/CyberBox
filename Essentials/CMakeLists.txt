cmake_minimum_required(VERSION 4.0)
project(Essentials C)

set(CMAKE_C_STANDARD 99)

add_executable(Essentials main.c
        sorts/sorts.c
        sorts/sorts.h
        compressor/compressor.c
        compressor/compressor.h
        decompressor/decompressor.c
        decompressor/decompressor.h
)
