cmake_minimum_required(VERSION 3.27)
project(lab_2_2 C)

set(CMAKE_C_STANDARD 11)

add_executable(lab_2_2_1
        fork.c
        pipe.c)
add_executable(lab_2_2_ex
        example.c)

add_executable(lab_2_2_2
        wait.c)
add_executable(lab_2_2_3 exec.c)
add_executable(lab_2_2_4 signal.c)
