cmake_minimum_required(VERSION 4.0)
project(DataManager C)

set(CMAKE_C_STANDARD 99)

add_executable(DataManager WIN32
        main.c
        actions.c
        actions.h
        resources.rc
)

target_link_options(DataManager PRIVATE -municode)