cmake_minimum_required(VERSION 3.16)
project(craft C)

set(CMAKE_C_STANDARD 99)

add_executable(craft main.c src/buffer.c src/buffer_dynamic.c src/buffer_socket.c)

target_include_directories(craft PRIVATE include)

if (WIN32)
    target_link_libraries(craft PRIVATE ws2_32.lib)
endif ()