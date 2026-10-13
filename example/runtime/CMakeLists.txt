cmake_minimum_required(VERSION 3.10.2)
project(oaax_example_runtime C)

set(CMAKE_C_STANDARD 11)
find_package(Threads REQUIRED)

# The runtime is a shared library implementing include/interface.h
add_library(RuntimeLibrary SHARED src/library.c)
target_include_directories(RuntimeLibrary PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_compile_options(RuntimeLibrary PRIVATE -Wall -Wextra)
target_link_libraries(RuntimeLibrary PRIVATE Threads::Threads)
