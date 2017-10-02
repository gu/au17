cmake_minimum_required(VERSION 3.8)
project(p2)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        common.h
        main.cc
        scanner.cc
        scanner.h
        token.h)

add_executable(p2 ${SOURCE_FILES})