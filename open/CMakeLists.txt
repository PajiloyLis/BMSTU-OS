cmake_minimum_required(VERSION 3.31)
project(open C)

set(CMAKE_C_STANDARD 11)

add_executable(first first.c)
add_executable(second second.c)
add_executable(first_multithread first_multithread.c)
add_executable(second_multithread second_multithread.c)
add_executable(third third.c)
add_executable(third_append third_append.c)
