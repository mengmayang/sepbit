cmake_minimum_required(VERSION 3.15)
project(sepbit)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")

add_subdirectory(src)
add_subdirectory(app)
