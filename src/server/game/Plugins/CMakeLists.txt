# TrinityCore Plugin System CMake Configuration
# This file integrates the plugin system with TrinityCore's build system

cmake_minimum_required(VERSION 3.16)

# Plugin system source files
set(PLUGIN_SYSTEM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/IPlugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PluginManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PluginManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PluginHooks.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PluginConfig.h
)

# Example plugin sources
set(EXAMPLE_PLUGIN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Examples/ExamplePlugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Examples/ExamplePlugin.cpp
)

# Plugin system library
add_library(plugin-system STATIC ${PLUGIN_SYSTEM_SOURCES})

# Set target properties
set_target_properties(plugin-system PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    FOLDER "server/game"
)

# Include directories for plugin system
target_include_directories(plugin-system PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/server/shared
    ${CMAKE_SOURCE_DIR}/src/server/shared/Configuration
    ${CMAKE_SOURCE_DIR}/src/server/shared/Logging
    ${CMAKE_SOURCE_DIR}/src/server/shared/Threading
    ${CMAKE_SOURCE_DIR}/src/server/shared/Utilities
    ${CMAKE_SOURCE_DIR}/src/server/game
    ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Player
    ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Object
    ${CMAKE_SOURCE_DIR}/src/server/game/World
    ${CMAKE_SOURCE_DIR}/src/server/game/Server
    ${CMAKE_SOURCE_DIR}/src/server/game/Chat
    ${CMAKE_SOURCE_DIR}/src/server/database
    ${CMAKE_SOURCE_DIR}/dep/fmt/include
)

# Link dependencies for plugin system
target_link_libraries(plugin-system
    shared
    game-interface
)

# Compiler definitions
target_compile_definitions(plugin-system PRIVATE
    TRINITY_API_USE_DYNAMIC_LINKING
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(plugin-system PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

# Example plugin library (optional)
option(BUILD_EXAMPLE_PLUGIN "Build example plugin" ON)

if(BUILD_EXAMPLE_PLUGIN)
    add_library(example-plugin SHARED ${EXAMPLE_PLUGIN_SOURCES})
    
    set_target_properties(example-plugin PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        FOLDER "server/plugins"
        PREFIX ""
        OUTPUT_NAME "ExamplePlugin"
    )
    
    # Include directories for example plugin
    target_include_directories(example-plugin PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/server/shared
        ${CMAKE_SOURCE_DIR}/src/server/shared/Configuration
        ${CMAKE_SOURCE_DIR}/src/server/shared/Logging
        ${CMAKE_SOURCE_DIR}/src/server/shared/Threading
        ${CMAKE_SOURCE_DIR}/src/server/shared/Utilities
        ${CMAKE_SOURCE_DIR}/src/server/game
        ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Player
        ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Object
        ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Item
        ${CMAKE_SOURCE_DIR}/src/server/game/World
        ${CMAKE_SOURCE_DIR}/src/server/game/Server
        ${CMAKE_SOURCE_DIR}/src/server/game/Chat
        ${CMAKE_SOURCE_DIR}/src/server/game/Globals
        ${CMAKE_SOURCE_DIR}/src/server/database
        ${CMAKE_SOURCE_DIR}/dep/fmt/include
    )
    
    # Link dependencies for example plugin
    target_link_libraries(example-plugin
        plugin-system
        shared
        game-interface
    )
    
    # Compiler definitions for example plugin
    target_compile_definitions(example-plugin PRIVATE
        TRINITY_API_USE_DYNAMIC_LINKING
    )
    
    # Platform-specific settings for example plugin
    if(WIN32)
        target_compile_definitions(example-plugin PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
        )
        
        # Export symbols for Windows DLL
        set_target_properties(example-plugin PROPERTIES
            WINDOWS_EXPORT_ALL_SYMBOLS ON
        )
    endif()
    
    # Install example plugin to plugins directory
    install(TARGETS example-plugin
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/plugins
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/plugins
    )
endif()

# Plugin discovery function
function(discover_plugins PLUGIN_DIR)
    file(GLOB_RECURSE PLUGIN_CONFIGS "${PLUGIN_DIR}/*/plugin.json")
    
    foreach(PLUGIN_CONFIG ${PLUGIN_CONFIGS})
        get_filename_component(PLUGIN_PATH ${PLUGIN_CONFIG} DIRECTORY)
        get_filename_component(PLUGIN_NAME ${PLUGIN_PATH} NAME)
        
        message(STATUS "Found plugin: ${PLUGIN_NAME} at ${PLUGIN_PATH}")
        
        # Check if plugin has CMakeLists.txt
        if(EXISTS "${PLUGIN_PATH}/CMakeLists.txt")
            add_subdirectory(${PLUGIN_PATH})
        else()
            message(WARNING "Plugin ${PLUGIN_NAME} does not have CMakeLists.txt")
        endif()
    endforeach()
endfunction()

# Auto-discover plugins in plugins directory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/plugins")
    discover_plugins("${CMAKE_CURRENT_SOURCE_DIR}/plugins")
endif()

# Create plugins directory structure
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/plugins")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/configs")

# Copy plugin configurations
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/configs")
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/configs/"
         DESTINATION "${CMAKE_BINARY_DIR}/plugins/configs/")
endif()

# Install plugin system headers for external plugin development
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/IPlugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PluginManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PluginHooks.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PluginConfig.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/trinitycore/plugins
)

# Install plugin system library
install(TARGETS plugin-system
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Create plugin development template
function(create_plugin_template PLUGIN_NAME)
    set(TEMPLATE_DIR "${CMAKE_BINARY_DIR}/plugin-template")
    file(MAKE_DIRECTORY "${TEMPLATE_DIR}")
    
    # Create basic plugin structure
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/templates/plugin.h.in"
        "${TEMPLATE_DIR}/${PLUGIN_NAME}.h"
        @ONLY
    )
    
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/templates/plugin.cpp.in"
        "${TEMPLATE_DIR}/${PLUGIN_NAME}.cpp"
        @ONLY
    )
    
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/templates/CMakeLists.txt.in"
        "${TEMPLATE_DIR}/CMakeLists.txt"
        @ONLY
    )
    
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/templates/plugin.json.in"
        "${TEMPLATE_DIR}/plugin.json"
        @ONLY
    )
    
    message(STATUS "Plugin template created at: ${TEMPLATE_DIR}")
endfunction()

# Macro for easy plugin creation
macro(add_trinitycore_plugin PLUGIN_NAME)
    set(options SHARED STATIC)
    set(oneValueArgs VERSION AUTHOR DESCRIPTION)
    set(multiValueArgs SOURCES DEPENDENCIES INCLUDE_DIRS LINK_LIBRARIES)
    
    cmake_parse_arguments(PLUGIN "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Default to shared library
    if(NOT PLUGIN_SHARED AND NOT PLUGIN_STATIC)
        set(PLUGIN_SHARED TRUE)
    endif()
    
    # Create plugin target
    if(PLUGIN_SHARED)
        add_library(${PLUGIN_NAME} SHARED ${PLUGIN_SOURCES})
    else()
        add_library(${PLUGIN_NAME} STATIC ${PLUGIN_SOURCES})
    endif()
    
    # Set target properties
    set_target_properties(${PLUGIN_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        FOLDER "server/plugins"
        PREFIX ""
    )
    
    # Include directories
    target_include_directories(${PLUGIN_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PLUGIN_INCLUDE_DIRS}
    )
    
    # Link libraries
    target_link_libraries(${PLUGIN_NAME}
        plugin-system
        ${PLUGIN_LINK_LIBRARIES}
    )
    
    # Install plugin
    if(PLUGIN_SHARED)
        install(TARGETS ${PLUGIN_NAME}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/plugins
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/plugins
        )
    endif()
endmacro()

# Print plugin system information
message(STATUS "TrinityCore Plugin System configured")
message(STATUS "  Plugin system library: plugin-system")
if(BUILD_EXAMPLE_PLUGIN)
    message(STATUS "  Example plugin: example-plugin")
endif()
message(STATUS "  Plugins directory: ${CMAKE_BINARY_DIR}/plugins")