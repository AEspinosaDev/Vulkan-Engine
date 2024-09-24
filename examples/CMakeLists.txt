
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
# --- Lighting scene setup ---
file(GLOB APPL_SOURCES
"lighting-test/*.cpp"
"lighting-test/*.h"
)
add_executable(LightingTest  ${APPL_SOURCES})

# Link projects against Engine lib
target_link_libraries(RendererApp PRIVATE VkFW)
target_link_libraries(RotatingKabuto PRIVATE VkFW)
target_link_libraries(LightingTest PRIVATE VkFW)

target_compile_definitions(VkFW PUBLIC EXAMPLES_RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")

set_property(TARGET RendererApp RotatingKabuto LightingTest PROPERTY FOLDER "examples")
