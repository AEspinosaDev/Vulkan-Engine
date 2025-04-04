
file(GLOB APP_SOURCES
"*.cpp"
"*.h"
)
add_executable(SkyTest  ${APP_SOURCES})
target_link_libraries(SkyTest PRIVATE VulkanEngine)



