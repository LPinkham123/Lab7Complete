cmake_minimum_required(VERSION 3.10)
project(Lab6_8 C)

set(CMAKE_C_STANDARD 17)

add_executable(Lab6_8 main.c my_string.c generic_vector.c associative_array.c)

configure_file(
        "${CMAKE_SOURCE_DIR}/dictionary_real.txt"
        "${CMAKE_BINARY_DIR}/dictionary_real.txt"
        COPYONLY
)