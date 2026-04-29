cmake_minimum_required(VERSION 3.10)
project(Sandpile)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_executable(sandpile bin/main.cpp)