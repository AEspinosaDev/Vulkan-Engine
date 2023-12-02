file(GLOB APP_SOURCES
"*.cpp"
"*.h"
)

add_executable(VKApp  ${APP_SOURCES})

# Link against Engine lib
target_link_libraries(VKApp PRIVATE VKEngine)

