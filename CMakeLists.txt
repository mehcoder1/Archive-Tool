cmake_minimum_required(VERSION 3.16)

project(archive_tool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(archive_tool
    src/main.cpp
    src/buffer.cpp
    src/archive.cpp
)

target_include_directories(archive_tool PRIVATE include)