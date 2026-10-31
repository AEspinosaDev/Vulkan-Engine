
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
add_subdirectory(thirdparty/logger)	
add_subdirectory(thirdparty/ImGuiFileDialog)	
target_link_libraries(ImGuiFileDialog PUBLIC imgui )

# Setup sources
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/add_module_files.cmake)
set(ENGINE_SOURCES "src/utils.cpp" "src/common.cpp")
set(ENGINE_HEADERS "")
add_module_files(core ENGINE_SOURCES ENGINE_HEADERS)
add_module_files(graphics ENGINE_SOURCES ENGINE_HEADERS)
add_module_files(render ENGINE_SOURCES ENGINE_HEADERS)
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
    logger
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

# Choose if building the project for distribution (before the examples, which add their own install rules)
option(DISTRIBUTION_MODE "Enable distribution mode (self-contained package assembled with cmake --install)" OFF)
if(DISTRIBUTION_MODE)
    message(STATUS "Distribution mode: package is assembled with cmake --install")

    # Application to package (setup/package.bat sets both)
    set(PACKAGE_TARGET "" CACHE STRING "Application packaged by cmake --install")
    set(PACKAGE_ASSETS "" CACHE PATH "Overrides the assets directory declared for PACKAGE_TARGET")

    # Define the macro for code (to be used in the engine)
    target_compile_definitions(VulkanEngine PUBLIC DISTRIBUTION_MODE=1)

    # Engine resources, found at runtime in <exe dir>/../resources
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/ DESTINATION resources)

    if(WIN32)
        # Shaders are compiled at runtime, so shaderc has to ship with the executables
        get_filename_component(VULKAN_SDK_DIR ${Vulkan_INCLUDE_DIR} DIRECTORY)
        install(FILES ${VULKAN_SDK_DIR}/Bin/shaderc_shared.dll DESTINATION bin)

        # App-local MSVC runtime, so the target machine does not need the VC++ Redistributable
        cmake_host_system_information(RESULT VS_DIR QUERY VS_17_DIR)
        file(GLOB MSVC_CRT_DIRS "${VS_DIR}/VC/Redist/MSVC/*/x64/Microsoft.VC*.CRT")
        if(MSVC_CRT_DIRS)
            list(SORT MSVC_CRT_DIRS COMPARE NATURAL ORDER DESCENDING)
            list(GET MSVC_CRT_DIRS 0 MSVC_CRT_DIR)
            install(FILES ${MSVC_CRT_DIR}/msvcp140.dll ${MSVC_CRT_DIR}/vcruntime140.dll ${MSVC_CRT_DIR}/vcruntime140_1.dll DESTINATION bin)
        else()
            message(WARNING "MSVC runtime DLLs not found, the package will need the VC++ Redistributable")
        endif()
    endif()
else()
    message(STATUS "Development mode: using resources in-place")

    # Define the macro for development mode
    target_compile_definitions(VulkanEngine PUBLIC DISTRIBUTION_MODE=0)
endif()
# Set the resource path to the local source directory
target_compile_definitions(VulkanEngine PUBLIC ENGINE_RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")

# Per-application assets directory, see vkfw_app_assets()
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/app_assets.cmake)

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

# Package one application: <package>/bin, <package>/resources (engine) and <package>/assets
if(DISTRIBUTION_MODE AND PACKAGE_TARGET)
    if(NOT TARGET ${PACKAGE_TARGET})
        message(FATAL_ERROR "PACKAGE_TARGET '${PACKAGE_TARGET}' is not a target of this project")
    endif()
    install(TARGETS ${PACKAGE_TARGET} RUNTIME DESTINATION bin)

    if(PACKAGE_ASSETS)
        get_filename_component(PACKAGE_ASSETS_DIR ${PACKAGE_ASSETS} ABSOLUTE BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    else()
        get_target_property(PACKAGE_ASSETS_DIR ${PACKAGE_TARGET} VKFW_ASSETS_DIR)
    endif()
    if(PACKAGE_ASSETS_DIR)
        message(STATUS "Packaging ${PACKAGE_TARGET} with assets from ${PACKAGE_ASSETS_DIR}")
        install(DIRECTORY ${PACKAGE_ASSETS_DIR}/ DESTINATION assets)
    else()
        message(STATUS "Packaging ${PACKAGE_TARGET} without assets")
    endif()
endif()

