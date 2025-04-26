
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
# if(UNIX)
#     set(SHADERC_INCLUDE_DIRS $ENV{VULKAN_SDK}/include)
#     set(SHADERC_LIBRARIES $ENV{VULKAN_SDK}/lib/libshaderc_combined.a)
# endif()

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
add_subdirectory(thirdparty/tinyxml)	
add_subdirectory(thirdparty/optick-1.4.0)	
add_subdirectory(thirdparty/ImGuiFileDialog)	
target_link_libraries(ImGuiFileDialog PUBLIC imgui )

# Setup sources
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/add_module_files.cmake)
set(ENGINE_SOURCES "src/utils.cpp")
set(ENGINE_HEADERS "")
add_module_files(core ENGINE_SOURCES ENGINE_HEADERS)
add_module_files(graphics ENGINE_SOURCES ENGINE_HEADERS)
add_module_files(systems ENGINE_SOURCES ENGINE_HEADERS)
add_module_files(tools ENGINE_SOURCES ENGINE_HEADERS)
add_library(VulkanEngine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})
# Include directories
target_include_directories(VulkanEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

# Configure vulkan path
link_directories(${VULKAN_SDK_ROOT})
# Link libraries
if (WIN32) 
    set(SHADERC_LIB "shaderc_shared")  
elseif(UNIX) 
    set(SHADERC_LIB "shaderc_combined.a")  
endif()
target_link_libraries(VulkanEngine PUBLIC 

    Vulkan::Vulkan 
    ${SHADERC_LIB}
    glm 
    glfw 
    stb_image 
    imgui 
    tiny_obj_loader 
    tinyply
    tinyxml
    optick
    ImGuiFileDialog)

# Set dependencies inside folder
set_property(TARGET glfw glm imgui stb_image tiny_obj_loader tinyply optick ImGuiFileDialog tinyxml PROPERTY FOLDER "thirdparty")

# For VISUAL STUDIO SLN EXPLORER ONLY !!!
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/group_sources.cmake)
create_source_group("Systems\\Renderers" "engine/systems/renderers" "systems/renderers")
create_source_group("Core" "engine/core" "core")
create_source_group("Tools" "engine/tools" "tools")
create_source_group("Graphics" "engine/graphics" "graphics")
create_source_group("Graphics\\Utilities" "engine/graphics/utilities" "graphics/utilities")
create_source_group("Core\\Passes" "engine/core/passes" "core/passes")
create_source_group("Core\\Scene Objects" "engine/core/scene" "core/scene")
create_source_group("Core\\Materials" "engine/core/materials" "core/materials")
create_source_group("Core\\Windows" "engine/core/windows" "core/windows")
create_source_group("Core\\Textures" "engine/core/textures" "core/textures")
create_source_group("Core\\Geometries" "engine/core/geometries" "core/geometries")
endif()

# Choose if building demos directory. User-Defined.
option(BUILD_EXAMPLES "Build Examples Directory" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/examples)
endif()
# Choose if building demos directory. User-Defined.
option(BUILD_TESTS "Build Tests Directory" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()

target_compile_definitions(VulkanEngine PUBLIC ENGINE_RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")

