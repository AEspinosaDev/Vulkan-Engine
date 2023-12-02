
cmake_minimum_required(VERSION 3.12)

project(VulkanEngine)

# Place inside source dir
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

#Activate all warnings

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # using Visual Studio C++
    message("Visual Studio C++")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # using GCC
    message("Using GCC")
endif()

# Set the platform (e.g., x64)
set(CMAKE_GENERATOR_PLATFORM x64) # or Win32, as needed

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For exporting compile config
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) #DOES NOT WORK WITH MSVC

# Find the Vulkan package
find_package(Vulkan REQUIRED)

if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found. Please install Vulkan SDK and ensure it's in your CMake search path.")
endif()

# Get the root path for Vulkan from the Vulkan_LIBRARY variable
get_filename_component(VULKAN_SDK_ROOT ${Vulkan_LIBRARY} DIRECTORY)

# Include CMake FetchContent
include(FetchContent)
# Set the directory where FetchContent will download and build content
set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/dependencies")

# GLFW
#Exclude addons
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.4 # Use the version you need
)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8 # Use the version you need
    CMAKE_ARGS -DGLM_TEST_ENABLE=OFF
)
FetchContent_MakeAvailable(glm)


# Configure vulkan path
link_directories(${VULKAN_SDK_ROOT})

add_subdirectory(${CMAKE_SOURCE_DIR}/core)
add_subdirectory(${CMAKE_SOURCE_DIR}/app)


set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# Set the output directory for debug build
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)
# Set the output directory for release build
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)
