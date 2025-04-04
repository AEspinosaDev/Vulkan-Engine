
message(STATUS "Building Tests Directory...")

add_subdirectory(procedural-sky)
add_subdirectory(skin)

target_compile_definitions(VulkanEngine PUBLIC TESTS_RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
set_property(TARGET SkyTest SkinTest PROPERTY FOLDER "tests")
