# TrinityCore Game Logic Components

# Game Library Sources
set(GAME_SOURCES
    # Core game files would go here
    # This is a placeholder structure
)

# Game Library Headers
set(GAME_HEADERS
    # Core game headers would go here
    # This is a placeholder structure
)

# Plugin System Integration
if(BUILD_MODULES)
    message(STATUS "Integrating plugin system with game logic")
    
    # Add plugin system sources
    list(APPEND GAME_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/Plugins/PluginManager.cpp
    )
    
    # Add plugin system headers
    list(APPEND GAME_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/Plugins/IPlugin.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Plugins/PluginManager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Plugins/PluginHooks.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Plugins/PluginConfig.h
    )
    
    # Include plugin directories
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/Plugins
        ${CMAKE_SOURCE_DIR}/modules
    )
    
    # Add plugin system subdirectory
    add_subdirectory(Plugins)
endif()

# Create game library
if(GAME_SOURCES)
    add_library(game STATIC ${GAME_SOURCES} ${GAME_HEADERS})
    
    target_include_directories(game PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/server/shared
        ${CMAKE_SOURCE_DIR}/src/server/database
    )
    
    target_link_libraries(game
        ${TRINITY_GLOBAL_LIBRARIES}
    )
    
    # Link plugin system if enabled
    if(BUILD_MODULES)
        target_link_libraries(game plugins)
        target_compile_definitions(game PRIVATE TRINITY_MODULES_ENABLED=1)
    endif()
    
    # Platform-specific settings
    if(WIN32)
        target_compile_definitions(game PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
    endif()
    
    # Set target properties
    set_target_properties(game PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        POSITION_INDEPENDENT_CODE ON
    )
else()
    # Create empty library for now
    add_library(game INTERFACE)
    
    target_include_directories(game INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    if(BUILD_MODULES)
        target_link_libraries(game INTERFACE plugins)
        target_compile_definitions(game INTERFACE TRINITY_MODULES_ENABLED=1)
    endif()
endif()

# Add subdirectories for game components
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Entities)
    add_subdirectory(Entities)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Maps)
    add_subdirectory(Maps)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Spells)
    add_subdirectory(Spells)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Combat)
    add_subdirectory(Combat)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/AI)
    add_subdirectory(AI)
endif()