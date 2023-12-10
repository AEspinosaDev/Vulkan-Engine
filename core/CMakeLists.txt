
file(GLOB LIB_SOURCES "src/*.cpp" "src/utilities/*.cpp" "src/scene_objects/*.cpp" "src/materials/*.cpp" "src/backend/*.cpp")

file(GLOB LIB_HEADERS "include/engine/*.h" "include/engine/utilities/*.h" "include/engine/scene_objects/*.h" "include/engine/materials/*.h")

file(GLOB LIB_INTERNAL_HEADERS "private/*.h")


# Add the library target
add_library(VKEngine STATIC ${LIB_SOURCES} ${LIB_HEADERS} ${LIB_INTERNAL_HEADERS})

# Set include directories for users of your library
target_include_directories(VKEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private
)

# Link Vulkan, glfw, shaderc and include glm
target_link_libraries(VKEngine Vulkan::Vulkan glfw shaderc_shared.lib)
target_include_directories(VKEngine PRIVATE ${glm_SOURCE_DIR})

# Set properties
set_target_properties(VKEngine PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


source_group("Source Files\\utilities" FILES ${UTIL_SOURCES})

# For VISUAL STUDIO SLN EXPLORER ONLY !!!
set(CORE_H include/engine/*.h)
set(UTIL_H include/engine/utilities/*.h)
set(MATERIAL_H include/engine/materials/*.h)
set(OBJECTS_H include/engine/scene_objects/*.h)
set(PRIVATE_H private/*.h)
source_group("Header Files\\utilities" FILES ${UTIL_H})
source_group("Header Files\\scene_objects" FILES ${OBJECTS_H})
source_group("Header Files\\materials" FILES ${MATERIAL_H})
source_group("Header Files\\" FILES ${CORE_H})
source_group("Header Files\\private" FILES ${PRIVATE_H})
set(CORE_SRC src/*.cpp)
set(UTIL_SRC src/utilities/*.cpp)
set(MATERIAL_SRC src/materials/*.cpp)
set(OBJECTS_SRC src/scene_objects/*.cpp)
set(BACKEND_SRC src/backend/*.cpp)
source_group("Source Files\\utilities" FILES ${UTIL_SRC})
source_group("Source Files\\scene_objects" FILES ${OBJECTS_SRC})
source_group("Source Files\\materials" FILES ${MATERIAL_SRC})
source_group("Source Files\\" FILES ${CORE_SRC})
source_group("Source Files\\backend" FILES ${BACKEND_SRC})


# Set configuration
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/")
set(TEXTURE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/textures/")
set(MODEL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/meshes/")
configure_file(config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/config.h)




