
cmake_minimum_required(VERSION 3.16)

project(VkFW)


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
add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/glm-1.0.1)
add_subdirectory(thirdparty/imgui) 
add_subdirectory(thirdparty/stb_image)
add_subdirectory(thirdparty/tiny_obj_loader)	
add_subdirectory(thirdparty/tinyply)	

# Setup
file(GLOB LIB_SOURCES 
    "src/core/*.cpp" 
    "src/core/scene/*.cpp"
    "src/core/materials/*.cpp" 
    "src/utilities/*.cpp" 
    "src/graphics/*.cpp"
    "src/graphics/renderpasses/*.cpp")
    
file(GLOB LIB_HEADERS 
    "include/VkFW/*.h"
    "include/VkFW/core/*.h" 
    "include/VkFW/core/scene/*.h" 
    "include/VkFW/core/materials/*.h" 
    "include/VkFW/utilities/*.h" 
    "include/VkFW/graphics/*.h"
    "include/VkFW/graphics/renderpasses/*.h")

add_library(VkFW STATIC ${LIB_SOURCES} ${LIB_HEADERS})

# Configure vulkan path
link_directories(${VULKAN_SDK_ROOT})
# Include directories
target_include_directories(VkFW PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
# Link libraries
target_link_libraries(VkFW PUBLIC 

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
        ${CMAKE_CURRENT_SOURCE_DIR}/include/VkFW/core/scene/*.h
    )
    file(GLOB materials_h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/VkFW/core/materials/*.h
    )
    file(GLOB utilities_h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/VkFW/utilities/*.h
    )
    file(GLOB core_h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/VkFW/core/*.h
    )
    file(GLOB backend_h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/VkFW/graphics/*.h
    )
    file(GLOB rpass_h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/VkFW/graphics/renderpasses/*.h
    )
    source_group("Header Files\\Utilities" FILES ${utilities_h})
    source_group("Header Files\\Core" FILES ${core_h})
    source_group("Header Files\\Core\\Scene Objects" FILES ${scene_objects_h})
    source_group("Header Files\\Core\\Materials" FILES ${materials_h})
    source_group("Header Files\\Graphics" FILES ${backend_h})
    source_group("Header Files\\Graphics\\RenderPasses" FILES ${rpass_h})
    
    file(GLOB scene_objects_cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/scene/*.cpp
    )
    file(GLOB materials_cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/materials/*.cpp
    )
    file(GLOB utilities_cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utilities/*.cpp
    )
    file(GLOB core_cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp
    )
    file(GLOB backend_cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/*.cpp
    )
    file(GLOB rpass_cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/renderpasses/*.cpp
    )
    
    source_group("Source Files\\Utilities" FILES ${utilities_cpp})
    source_group("Source Files\\Core" FILES ${core_cpp})
    source_group("Source Files\\Core\\Scene Objects" FILES ${scene_objects_cpp})
    source_group("Source Files\\Core\\Materials" FILES ${materials_cpp})
    source_group("Source Files\\Graphics" FILES ${backend_cpp})
    source_group("Source Files\\Graphics\\RenderPasses" FILES ${rpass_cpp})

endif()

# Choose if building demos directory. User-Defined.
option(BUILD_EXAMPLES "Build Examples Directory" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(${CMAKE_SOURCE_DIR}/examples)
endif()

target_compile_definitions(VkFW PUBLIC ENGINE_RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")

