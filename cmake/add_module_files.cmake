
function(add_module_files module_name sources headers)
    
    file(GLOB_RECURSE MODULE_SOURCES 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/${module_name}/**/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/${module_name}/*.cpp"
    )
    
    
    file(GLOB_RECURSE MODULE_HEADERS 
        "${CMAKE_CURRENT_SOURCE_DIR}/include/engine/${module_name}/**/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/engine/${module_name}/*.h"
    )
    
    # Append the files to the global lists
    set(${sources} ${${sources}} ${MODULE_SOURCES} PARENT_SCOPE)
    set(${headers} ${${headers}} ${MODULE_HEADERS} PARENT_SCOPE)
endfunction()