function(create_source_group group_name header_dir source_dir)

    file(GLOB headers
        ${CMAKE_CURRENT_SOURCE_DIR}/include/${header_dir}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/${header_dir}/*.h
    )
    source_group("Header Files\\${group_name}" FILES ${headers})

    
    file(GLOB sources
        ${CMAKE_CURRENT_SOURCE_DIR}/src/${source_dir}/*.cpp
    )
    source_group("Source Files\\${group_name}" FILES ${sources})

    # Optionally, return headers and sources to be added to a target
    # set(SOURCE_FILES ${headers} ${sources} PARENT_SCOPE)
endfunction()