
message(STATUS "Building Demos Directory...")

#DEMO 1
# --- Complex renderer application ---
file(GLOB APP_SOURCES
"renderer-app/*.cpp"
"renderer-app/*.h"
)
add_executable(RendererApp  ${APP_SOURCES})

#DEMO 2
# --- Kabuto rotating ---
add_executable(RotatingKabuto "rotating-kabuto/main.cpp" )

#DEMO 3
# --- Simple scene setup ---
# add_executable(SampleScene  ${APP_SOURCES})

# Link projects against Engine lib
target_link_libraries(RendererApp PRIVATE VulkanEngine)
target_link_libraries(RotatingKabuto PRIVATE VulkanEngine)



# Resource path resolve and configuration
set(DEMO_TEXTURES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources/textures/")
set(DEMO_MODELS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources/meshes/")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)
