# TrinityCore Server Components

# Server Libraries
set(SERVER_LIBRARIES
    shared
    game
    worldserver
    authserver
)

# Add shared components
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/shared)
    add_subdirectory(shared)
endif()

# Add database components
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/database)
    add_subdirectory(database)
endif()

# Add game logic
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/game)
    add_subdirectory(game)
endif()

# Add world server
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/worldserver)
    add_subdirectory(worldserver)
endif()

# Add auth server
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/authserver)
    add_subdirectory(authserver)
endif()

# Add scripts
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/scripts)
    add_subdirectory(scripts)
endif()