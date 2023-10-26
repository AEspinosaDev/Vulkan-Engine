
cmake_minimum_required(VERSION 3.12)


project(VulkanEngine)

# Place inside source dir
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

# if(WIN32 OR UNIX)
#     # Your Windows and Linux-specific configuration goes here
# else()
#     message(FATAL_ERROR "This project is intended for Windows and Linux.")
# endif()

# Set the CMake generator for Visual Studio 2022
set(CMAKE_GENERATOR "Visual Studio 17 2022" CACHE STRING "Visual Studio 2022" FORCE)

# Set the platform (e.g., x64)
set(CMAKE_GENERATOR_PLATFORM x64) # or Win32, as needed

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/third_party")

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


# Configure Shaderc
#link_directories(C:/VulkanSDK/1.3.261.1/Lib)  # Path to where shaderc_shared.lib is located
link_directories(${VULKAN_SDK_ROOT})

file(GLOB SOURCES
"src/*.cpp"
"src/*.h"
)

# Add your source files
add_executable(VulkanEngine ${SOURCES})

# Link Vulkan, glfw, and include glm
target_link_libraries(VulkanEngine Vulkan::Vulkan glfw)
target_include_directories(VulkanEngine PRIVATE ${glm_SOURCE_DIR})

# Link against Shaderc
target_link_libraries(VulkanEngine shaderc_shared.lib)

# Set the output directory for the executable (optional)
# set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
