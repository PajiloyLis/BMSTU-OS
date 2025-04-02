cmake_minimum_required(VERSION 3.27)
project(rpc C)

set(CMAKE_C_STANDARD 11)

include_directories(. /usr/include/tirpc/)

add_executable(rpc
        example/calculator_clnt.c
        example/calculator_client.c
        example/calculator_server.c
        example/calculator_xdr.c
        example/calculator_svc.c
)

