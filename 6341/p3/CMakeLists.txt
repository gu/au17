cmake_minimum_required(VERSION 3.8)
project(p3)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        common.h
        main.cc
        scanner.cc
        scanner.h
        token.h node.h parser.h parser.cc)

add_executable(p3 ${SOURCE_FILES})