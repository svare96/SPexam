cmake_minimum_required(VERSION 3.25)
project(Exam)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

add_executable(Exam main.cpp lib/lib.cpp lib/lib.h lib/graph.cpp lib/graph.h)
