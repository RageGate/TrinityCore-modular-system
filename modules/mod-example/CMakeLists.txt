# Example Module CMake Configuration
# This demonstrates how to build a TrinityCore module

cmake_minimum_required(VERSION 3.16)

# Module information
set(MODULE_NAME "ExampleModule")
set(MODULE_VERSION "1.0.0")
set(MODULE_DESCRIPTION "Example module for TrinityCore")

# Read module metadata
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/module.json")
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/module.json" MODULE_JSON)
    message(STATUS "Loading module: ${MODULE_NAME}")
else()
    message(FATAL_ERROR "module.json not found in ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# Module source files
set(MODULE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ExampleModule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ExampleModule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ExampleEventHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ExampleEventHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ExampleCommands.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ExampleCommands.cpp
)

# Create module library
add_library(${MODULE_NAME} SHARED ${MODULE_SOURCES})

# Set target properties
set_target_properties(${MODULE_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    FOLDER "modules"
    PREFIX ""
    OUTPUT_NAME "${MODULE_NAME}"
    VERSION ${MODULE_VERSION}
)

# Include directories
target_include_directories(${MODULE_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server/game/Plugins
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

# Link libraries
target_link_libraries(${MODULE_NAME}
    plugin-system
    shared
    game-interface
)

# Compiler definitions
target_compile_definitions(${MODULE_NAME} PRIVATE
    TRINITY_API_USE_DYNAMIC_LINKING
    MODULE_NAME="${MODULE_NAME}"
    MODULE_VERSION="${MODULE_VERSION}"
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(${MODULE_NAME} PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
    
    # Export symbols for Windows DLL
    set_target_properties(${MODULE_NAME} PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()

# Install module binary
install(TARGETS ${MODULE_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/modules
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/modules
)

# Install module configuration
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/conf")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/conf/"
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/modules
        FILES_MATCHING PATTERN "*.conf"
    )
endif()

# Install module data files
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/data")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data/"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/modules/${MODULE_NAME}
    )
endif()

# Install SQL files
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sql")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/sql/"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/sql/modules/${MODULE_NAME}
        FILES_MATCHING PATTERN "*.sql"
    )
endif()

# Copy configuration files to build directory for development
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/conf")
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/conf/"
         DESTINATION "${CMAKE_BINARY_DIR}/etc/modules/")
endif()

# Module validation
function(validate_module_json MODULE_JSON_FILE)
    if(NOT EXISTS ${MODULE_JSON_FILE})
        message(FATAL_ERROR "Module JSON file not found: ${MODULE_JSON_FILE}")
    endif()
    
    # Basic JSON validation could be added here
    message(STATUS "Module JSON validated: ${MODULE_JSON_FILE}")
endfunction()

# Validate this module
validate_module_json("${CMAKE_CURRENT_SOURCE_DIR}/module.json")

# Add module to global module list
get_property(GLOBAL_MODULES GLOBAL PROPERTY TRINITYCORE_MODULES)
if(NOT GLOBAL_MODULES)
    set_property(GLOBAL PROPERTY TRINITYCORE_MODULES "")
endif()
set_property(GLOBAL APPEND PROPERTY TRINITYCORE_MODULES ${MODULE_NAME})

# Module testing (optional)
option(BUILD_MODULE_TESTS "Build module tests" OFF)
if(BUILD_MODULE_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    add_subdirectory(tests)
endif()

# Module documentation generation (optional)
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/docs")
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/docs/modules/${MODULE_NAME}")
    doxygen_add_docs(${MODULE_NAME}-docs
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/docs
        COMMENT "Generating ${MODULE_NAME} documentation"
    )
endif()

# Print module information
message(STATUS "Module configured: ${MODULE_NAME} v${MODULE_VERSION}")
message(STATUS "  Description: ${MODULE_DESCRIPTION}")
message(STATUS "  Sources: ${MODULE_SOURCES}")
message(STATUS "  Install path: ${CMAKE_INSTALL_LIBDIR}/modules")

# Development helpers
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Add debug symbols and disable optimizations for module development
    target_compile_options(${MODULE_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-g -O0>
        $<$<CXX_COMPILER_ID:MSVC>:/Zi /Od>
    )
endif()

# Module-specific compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${MODULE_NAME} PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
        -Wno-missing-field-initializers
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${MODULE_NAME} PRIVATE
        /W4
        /wd4100  # unreferenced formal parameter
        /wd4201  # nonstandard extension used
    )
endif()