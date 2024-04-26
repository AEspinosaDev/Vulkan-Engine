cmake_minimum_required(VERSION 3.1)
project(tinyply)

add_library(tinyply)
target_sources(tinyply PRIVATE "src/tinyply.cpp")
target_include_directories(tinyply PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")