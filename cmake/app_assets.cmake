# Declares the assets directory an application loads at runtime through GET_ASSET_PATH().
# Development builds read it in place; distribution packages ship it as <package>/assets.
function(vkfw_app_assets target directory)
    get_filename_component(directory ${directory} ABSOLUTE)
    target_compile_definitions(${target} PRIVATE APP_ASSETS_PATH="${directory}/")
    set_target_properties(${target} PROPERTIES VKFW_ASSETS_DIR ${directory})
endfunction()
