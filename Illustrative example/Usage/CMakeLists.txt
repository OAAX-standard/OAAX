cmake_minimum_required(VERSION 3.10.2)
project(example)

# Shared Library
add_executable(main main.c artifacts/interface.h)
# Include the interface file: interface.h
target_include_directories(main PUBLIC artifacts)