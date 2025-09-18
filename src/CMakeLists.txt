# TrinityCore Source Directory

# Add server subdirectory
add_subdirectory(server)

# Add shared libraries if they exist
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/shared)
    add_subdirectory(shared)
endif()

# Add common utilities if they exist
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/common)
    add_subdirectory(common)
endif()

# Add tools if they exist
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tools)
    add_subdirectory(tools)
endif()