cmake_minimum_required(VERSION 3.6)
project(networks)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -static-libgcc -static-libstdc++ -pthread")

set(SOURCE_FILES sources/main.cpp headers/Server.h sources/Server.cpp sources/Command.cpp headers/Command.h sources/ServerCommand.cpp headers/ServerCommand.h headers/ClientCommand.h)
add_executable(networks ${SOURCE_FILES})