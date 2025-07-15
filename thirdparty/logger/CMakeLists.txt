cmake_minimum_required(VERSION 3.6)
project(logger)


add_library(logger INTERFACE)

target_include_directories(logger INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
