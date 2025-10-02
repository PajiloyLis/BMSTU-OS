cmake_minimum_required(VERSION 3.27)
project(socket C)

set(CMAKE_C_STANDARD 11)

add_executable(server_no_response server_no_response.c)

add_executable(client_no_response client_no_response.c
        server_response.c
        request_response.h)

add_executable(socketpair socketpair.c)
add_executable(client_response client_response.c)
