cmake_minimum_required(VERSION 3.27)
project(socket_remote C)

set(CMAKE_C_STANDARD 11)

add_executable(socket_remote process_server.c
        process_client.c
        thread_server.c
        thread_client.c)

add_executable(thread_server thread_server.c)

target_link_libraries(thread_server pthread)

add_executable(thread_client thread_client.c)
add_executable(epoll_server epoll_server.c
        epoll_server.c)
add_executable(epoll_client epoll_client.c)
