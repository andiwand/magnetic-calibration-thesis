cmake_minimum_required(VERSION 3.12.4)
project(magnetics CXX)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")

add_subdirectory(3rdparty)

add_subdirectory(protocol)
add_subdirectory(pipeline)
add_subdirectory(magnetics)
add_subdirectory(cli)

add_subdirectory(test)
