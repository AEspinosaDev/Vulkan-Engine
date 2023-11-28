
file(GLOB SOURCES
"*.cpp"
"*.h"
"internal/*.cpp"
"internal/*.h"
"utilities/*.h"
"utilities/*.cpp"
)

# Add your source files
add_library(VulkanEngine STATIC ${SOURCES})
target_include_directories(VulkanEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link Vulkan, glfw, shaderc and include glm
target_link_libraries(VulkanEngine Vulkan::Vulkan glfw shaderc_shared.lib)
target_include_directories(VulkanEngine PRIVATE ${glm_SOURCE_DIR})


# Set properties
set_target_properties(VulkanEngine PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

