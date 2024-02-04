

file(GLOB APP_SOURCES
"*.h"
"src/*.cpp"
"include/demo/*.h"
)

add_executable(VKApp  ${APP_SOURCES})

# Link against Engine lib
target_link_libraries(VKApp PRIVATE VKEngine)
target_include_directories(VKApp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include )


# Configuration
set(TEXTURE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/textures/")
set(MODEL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/meshes/")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/demo/config.h)
