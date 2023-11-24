file(GLOB APP_SOURCES
"*.cpp"
"*.h"
)

add_executable(VulkanApp  ${APP_SOURCES})

# Link against Engine lib
target_link_libraries(VulkanApp PRIVATE VulkanEngine)

