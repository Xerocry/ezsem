cmake_minimum_required(VERSION 3.5)
project(networks)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -static-libgcc -static-libstdc++")

set(SOURCE_FILES sources/main.cpp headers/Client.h sources/Client.cpp)

add_executable(networks ${SOURCE_FILES})

if(WIN32)
    target_link_libraries(networks advapi32 mswsock wsock32 ws2_32)
endif()