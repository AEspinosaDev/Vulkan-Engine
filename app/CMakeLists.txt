file(GLOB APP_SOURCES
"*.h"
"src/*.cpp"
"src/*.h"
)

add_executable(VKApp  ${APP_SOURCES})

# Link against Engine lib
target_link_libraries(VKApp PRIVATE VKEngine)

# Configuration
set(TEXTURE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/textures/")
set(MODEL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/meshes/")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)
