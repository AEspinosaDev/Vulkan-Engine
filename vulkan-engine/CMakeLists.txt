
file(GLOB LIB_SOURCES "src/*.cpp" "src/utilities/*.cpp" "src/scene_objects/*.cpp" "src/materials/*.cpp" "src/backend/*.cpp"  "third_party/imgui/*.cpp")

file(GLOB LIB_HEADERS "include/engine/*.h" "include/engine/utilities/*.h" "include/engine/scene_objects/*.h" "include/engine/materials/*.h" "include/engine/backend/*.h")

file(GLOB LIB_INTERNAL_HEADERS "third_party/imgui/*.h")


# Add the library target
add_library(VKEngine STATIC ${LIB_SOURCES} ${LIB_HEADERS} ${LIB_INTERNAL_HEADERS})

# Set include directories for users of your library
target_include_directories(VKEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyobjloader
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb_image
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyply
    PRIVATE
        ${glm_SOURCE_DIR}
)

# Link Vulkan, glfw, shaderc and include glm
target_link_libraries(VKEngine Vulkan::Vulkan glfw shaderc_shared.lib)

# Set properties
set_target_properties(VKEngine PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


# For VISUAL STUDIO SLN EXPLORER ONLY !!!
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    file(GLOB scene_objects_h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/scene_objects/*.h
    )
    file(GLOB materials_h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/materials/*.h
    )
    file(GLOB utilities_h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/utilities/*.h
    )
    file(GLOB core_h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/*.h
    )
    file(GLOB backend_h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/backend/*.h
    )
    source_group("Header Files\\Utilities" FILES ${utilities_h})
    source_group("Header Files\\Scene Objects" FILES ${scene_objects_h})
    source_group("Header Files\\Materials" FILES ${materials_h})
    source_group("Header Files\\" FILES ${core_h})
    source_group("Header Files\\Backend" FILES ${backend_h})
    
    
    file(GLOB scene_objects_cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/scene_objects/*.cpp
    )
    file(GLOB materials_cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/materials/*.cpp
    )
    file(GLOB utilities_cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utilities/*.cpp
    )
    file(GLOB core_cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    )
    file(GLOB backend_cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backend/*.cpp
    )
    
    source_group("Source Files\\Utilities" FILES ${utilities_cpp})
    source_group("Source Files\\Scene Objects" FILES ${scene_objects_cpp})
    source_group("Source Files\\Materials" FILES ${materials_cpp})
    source_group("Source Files\\" FILES ${core_cpp})
    source_group("Source Files\\Backend" FILES ${backend_cpp})

    file(GLOB deps_
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyobjloader/*h
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyply/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb_image/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/*.cpp
    )
    source_group("Third Party" FILES ${deps_})
endif()

# Set configuration
set(VK_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/")
set(VK_TEXTURE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/textures/")
set(VK_MODEL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/meshes/")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/vk_config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/vk_config.h)




