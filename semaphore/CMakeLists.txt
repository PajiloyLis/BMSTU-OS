cmake_minimum_required(VERSION 3.27)
project(semaphore C)

set(CMAKE_C_STANDARD 11)

add_executable(consumer_producer consumer_producer.c)
add_executable(reader_writer
        reader_writer.c)
