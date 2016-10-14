cmake_minimum_required(VERSION 3.5)
project(client)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -static-libgcc -static-libstdc++ -pthread")
set(SOURCE_FILES sources/main.cpp headers/Client.h sources/Client.cpp)

add_executable(client ${SOURCE_FILES})

#target_link_libraries(client advapi32 mswsock wsock32 ws2_32)