
message(STATUS "Building Examples Directory...")

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
# --- Raytracing Example ---
file(GLOB APPRT_SOURCES
"raytracing/*.cpp"
"raytracing/*.h"
)
add_executable(RaytracingApp  ${APPRT_SOURCES})

#DEMO 4
# --- Sponza Example ---
file(GLOB SPONZA_SOURCES
"sponza/*.cpp"
"sponza/*.h"
)
add_executable(SponzaApp  ${SPONZA_SOURCES})

# Link projects against Engine lib
target_link_libraries(RendererApp PRIVATE VulkanEngine)
target_link_libraries(RotatingKabuto PRIVATE VulkanEngine)
target_link_libraries(RaytracingApp PRIVATE VulkanEngine)
target_link_libraries(SponzaApp PRIVATE VulkanEngine)

target_compile_definitions(VulkanEngine PUBLIC EXAMPLES_RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")

set_property(TARGET RendererApp RotatingKabuto RaytracingApp SponzaApp PROPERTY FOLDER "examples")
