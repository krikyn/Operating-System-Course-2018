cmake_minimum_required(VERSION 3.8)
project(server)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS -pthread)
set(SOURCE_FILES server.c)

add_executable(server ${SOURCE_FILES})

