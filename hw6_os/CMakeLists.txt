cmake_minimum_required(VERSION 3.26)
project(hw6_os C)

set(CMAKE_C_STANDARD 11)

add_executable(hw6_os fileops-test.c
        fileops.atlibby.alynkirscht.h
        fileops.atlibby.alynkirscht.c)
