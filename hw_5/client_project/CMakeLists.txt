cmake_minimum_required(VERSION 3.8)
project(client)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES client.c)
add_executable(client ${SOURCE_FILES})