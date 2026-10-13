cmake_minimum_required(VERSION 3.10.2)
project(example)

# Shared Library
add_library(RuntimeLibrary SHARED src/library.c
        include/interface.h)
# Include the interface file: interface.h
target_include_directories(RuntimeLibrary PUBLIC include)