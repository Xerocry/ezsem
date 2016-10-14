cmake_minimum_required(VERSION 3.6)
project(server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -static-libgcc -static-libstdc++ -pthread")
set(SOURCE_FILES sources/main.cpp headers/Server.h sources/Server.cpp sources/Command.cpp headers/Command.h sources/ServerCommand.cpp headers/ServerCommand.h headers/ClientCommand.h sources/ClientCommand.cpp headers/Global.h)

add_executable(server ${SOURCE_FILES})

#target_link_libraries(server advapi32 mswsock wsock32 ws2_32)