


file(GLOB APP_SOURCES
"*.cpp"
"*.h"
)
add_executable(HeadlessTest  ${APP_SOURCES})
target_link_libraries(HeadlessTest PRIVATE VulkanEngine)
add_test(NAME RUNTIME_OUTPUT_NAMEHeadlessTest COMMAND HeadlessTest)



