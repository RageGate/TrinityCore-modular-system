cmake_minimum_required(VERSION 3.16)

# Project Configuration
project(TrinityCore
    VERSION 3.3.5
    DESCRIPTION "TrinityCore World of Warcraft Server Emulator"
    LANGUAGES CXX C
)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build Configuration
set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")

# Platform Detection
if(WIN32)
    set(TRINITY_PLATFORM "Windows")
elseif(APPLE)
    set(TRINITY_PLATFORM "macOS")
else()
    set(TRINITY_PLATFORM "Linux")
endif()

# Output Directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Global Compiler Options
if(MSVC)
    add_compile_options(/W3 /permissive-)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wno-unused-parameter)
endif()

# Find Required Packages
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem thread program_options iostreams regex)
find_package(MySQL REQUIRED)
find_package(OpenSSL REQUIRED)

# Global Include Directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/shared
    ${CMAKE_SOURCE_DIR}/src/server/game
    ${CMAKE_SOURCE_DIR}/dep/include
    ${Boost_INCLUDE_DIRS}
    ${MYSQL_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
)

# Global Link Libraries
set(TRINITY_GLOBAL_LIBRARIES
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${MYSQL_LIBRARY}
    ${OPENSSL_LIBRARIES}
)

# Module System Configuration
option(BUILD_MODULES "Build modular system" ON)
option(BUILD_EXAMPLE_MODULES "Build example modules" ON)
option(MODULE_HOT_RELOAD "Enable module hot reload" OFF)

# Module Registry
set(TRINITY_MODULE_REGISTRY "${CMAKE_BINARY_DIR}/module_registry.json")
set(TRINITY_MODULE_CONFIG_DIR "${CMAKE_BINARY_DIR}/etc/modules")

# Create module configuration directory
file(MAKE_DIRECTORY ${TRINITY_MODULE_CONFIG_DIR})

# Add subdirectories
add_subdirectory(src)

# Add modules directory if enabled
if(BUILD_MODULES)
    message(STATUS "Building with modular system support")
    add_subdirectory(modules)
else()
    message(STATUS "Building without modular system")
endif()

# Installation Configuration
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation directory")

install(DIRECTORY ${CMAKE_SOURCE_DIR}/sql/
    DESTINATION share/trinitycore/sql
    FILES_MATCHING PATTERN "*.sql"
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/
    DESTINATION share/trinitycore/data
    OPTIONAL
)

# Module installation
if(BUILD_MODULES)
    install(FILES ${TRINITY_MODULE_REGISTRY}
        DESTINATION etc/trinitycore
        OPTIONAL
    )
    
    install(DIRECTORY ${TRINITY_MODULE_CONFIG_DIR}/
        DESTINATION etc/trinitycore/modules
        OPTIONAL
    )
endif()

# Custom Targets
add_custom_target(clean-all
    COMMAND ${CMAKE_BUILD_TOOL} clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMENT "Cleaning all build files"
)

# Development helpers
if(BUILD_MODULES)
    add_custom_target(list-modules
        COMMAND ${CMAKE_COMMAND} -E echo "Available modules:"
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/modules/list_modules.cmake
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Listing available modules"
    )
    
    add_custom_target(validate-modules
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/modules/validate_modules.cmake
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Validating module configurations"
    )
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "TrinityCore Configuration Summary:")
message(STATUS "  Platform: ${TRINITY_PLATFORM}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Modules: ${BUILD_MODULES}")
if(BUILD_MODULES)
    message(STATUS "  Example Modules: ${BUILD_EXAMPLE_MODULES}")
    message(STATUS "  Hot Reload: ${MODULE_HOT_RELOAD}")
endif()
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")