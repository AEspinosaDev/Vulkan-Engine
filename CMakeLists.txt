
cmake_minimum_required(VERSION 3.16)

project(VulkanEngine)

# Place inside source dir
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

# Activate all warnings
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

# Set up dependencies
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/glfw-3.4)
add_subdirectory(thirdparty/glm-1.0.1)
add_subdirectory(thirdparty/imgui) 
add_subdirectory(thirdparty/stb_image)
add_subdirectory(thirdparty/tiny_obj_loader)	
add_subdirectory(thirdparty/tinyply)	

# Setup
file(GLOB LIB_SOURCES 
    "src/core/*.cpp" 
    "src/utilities/*.cpp" 
    "src/scene/*.cpp"
    "src/materials/*.cpp" 
    "src/backend/*.cpp"
    "src/renderpasses/*.cpp" )
    
file(GLOB LIB_HEADERS 
    "include/engine/*.h"
    "include/engine/core/*.h" 
    "include/engine/utilities/*.h" 
    "include/engine/scene/*.h" 
    "include/engine/materials/*.h" 
    "include/engine/backend/*.h"
    "include/engine/renderpasses/*.h")

add_library(VulkanEngine STATIC ${LIB_SOURCES} ${LIB_HEADERS})

# Configure vulkan path
link_directories(${VULKAN_SDK_ROOT})
# Include directories
target_include_directories(VulkanEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
# Link libraries
target_link_libraries(VulkanEngine PUBLIC 

    Vulkan::Vulkan 
    shaderc_shared.lib 
    glm 
    glfw 
    stb_image 
    imgui 
    tiny_obj_loader 
    tinyply)

# Set dependencies inside folder
set_property(TARGET glfw glm imgui stb_image tiny_obj_loader tinyply PROPERTY FOLDER "thirdparty")

# For VISUAL STUDIO SLN EXPLORER ONLY !!!
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    file(GLOB scene_objects_h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/scene/*.h
    )
    file(GLOB materials_h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/materials/*.h
    )
    file(GLOB utilities_h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/utilities/*.h
    )
    file(GLOB core_h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/core/*.h
    )
    file(GLOB backend_h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/backend/*.h
    )
    file(GLOB rpass_h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/renderpasses/*.h
    )
    source_group("Header Files\\Utilities" FILES ${utilities_h})
    source_group("Header Files\\Scene Objects" FILES ${scene_objects_h})
    source_group("Header Files\\Materials" FILES ${materials_h})
    source_group("Header Files\\Core" FILES ${core_h})
    source_group("Header Files\\Backend" FILES ${backend_h})
    source_group("Header Files\\RenderPasses" FILES ${rpass_h})
    
    
    file(GLOB scene_objects_cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/scene/*.cpp
    )
    file(GLOB materials_cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/materials/*.cpp
    )
    file(GLOB utilities_cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utilities/*.cpp
    )
    file(GLOB core_cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp
    )
    file(GLOB backend_cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/backend/*.cpp
    )
    file(GLOB rpass_cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderpasses/*.cpp
    )
    
    source_group("Source Files\\Utilities" FILES ${utilities_cpp})
    source_group("Source Files\\Scene Objects" FILES ${scene_objects_cpp})
    source_group("Source Files\\Materials" FILES ${materials_cpp})
    source_group("Source Files\\Core" FILES ${core_cpp})
    source_group("Source Files\\Backend" FILES ${backend_cpp})
    source_group("Source Files\\RenderPasses" FILES ${rpass_cpp})

endif()

# Choose if building demos directory. User-Defined.
option(BUILD_EXAMPLES "Build Examples Directory" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(${CMAKE_SOURCE_DIR}/examples)
endif()

target_compile_definitions(VulkanEngine PUBLIC ENGINE_RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")

