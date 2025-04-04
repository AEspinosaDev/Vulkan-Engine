
file(GLOB APP_SOURCES
"*.cpp"
"*.h"
)
add_executable(SkinTest  ${APP_SOURCES})
target_link_libraries(SkinTest PRIVATE VulkanEngine)



