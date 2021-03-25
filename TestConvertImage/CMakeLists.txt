cmake_minimum_required(VERSION 3.0)
project(ConvertImage VERSION 0.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lGL")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lGLU")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lglfw")

add_executable(convertImage convertImage.cpp)