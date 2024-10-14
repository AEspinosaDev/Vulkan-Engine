cmake_minimum_required(VERSION 3.1)
project(optick)

file(GLOB OP_SOURCES 
    "src/*.cpp" )

add_library(optick)
target_sources(optick PRIVATE ${OP_SOURCES})
target_include_directories(optick PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/")
target_link_libraries(optick PUBLIC Vulkan::Vulkan)