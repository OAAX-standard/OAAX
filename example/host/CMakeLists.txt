cmake_minimum_required(VERSION 3.10.2)
project(oaax_example_host C)

set(CMAKE_C_STANDARD 11)

# The host only needs the header: the runtime library is loaded at run time with dlopen
add_executable(host main.c)
target_include_directories(host PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_compile_options(host PRIVATE -Wall -Wextra)
target_link_libraries(host PRIVATE ${CMAKE_DL_LIBS})
