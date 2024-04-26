cmake_minimum_required(VERSION 3.1)
project(tiny_obj_loader)

add_library(tiny_obj_loader)
target_sources(tiny_obj_loader PRIVATE "src/tiny_obj_loader.cpp")
target_include_directories(tiny_obj_loader PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")