cmake_minimum_required(VERSION 3.1)
project(tinyxml)

add_library(tinyxml)
target_sources(tinyxml PRIVATE "src/tinyxml2.cpp")
target_include_directories(tinyxml PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")