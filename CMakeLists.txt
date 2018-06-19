cmake_minimum_required(VERSION 3.10)
project(server_new)

set(CMAKE_CXX_STANDARD 11)

add_executable(server_new main.cpp config.cpp config.h
        log.cpp log.h server.cpp server.h debug.h MMech.cpp MMech.h device.cpp device.h client.cpp client.h)

if(CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_definitions(server PUBLIC NDEBUG)
endif()