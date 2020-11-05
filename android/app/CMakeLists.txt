cmake_minimum_required(VERSION 3.12.4)
project(magnetics-jni CXX)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D DEBUG=1 -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D DEBUG=1 -O3")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -D NDEBUG=1 -O3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D NDEBUG=1 -O3")

list(APPEND CMAKE_FIND_ROOT_PATH
        ${CMAKE_CURRENT_LIST_DIR}/boost/arm64-v8a
        ${CMAKE_CURRENT_LIST_DIR}/boost/armeabi-v7a
        ${CMAKE_CURRENT_LIST_DIR}/boost/x86
        ${CMAKE_CURRENT_LIST_DIR}/boost/x86_64
        )
find_package(Boost 1.73.0 REQUIRED)

add_subdirectory(../../core core)

add_library(magnetics-jni SHARED
        src/main/cpp/jni.cpp
        )
target_include_directories(magnetics-jni
        PRIVATE
        src/main/cpp
        )
target_link_libraries(magnetics-jni
        PRIVATE
        magnetics
        protocol
        log
        )
