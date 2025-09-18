#
# TrinityCore Modules Build Configuration
# This file manages the build process for all modules in the modules directory
#

cmake_minimum_required(VERSION 3.16)

# Module system configuration
set(MODULES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(MODULES_OUTPUT_DIR "${CMAKE_BINARY_DIR}/modules")
set(MODULES_CONFIG_DIR "${CMAKE_BINARY_DIR}/etc/modules")

# Create output directories
file(MAKE_DIRECTORY "${MODULES_OUTPUT_DIR}")
file(MAKE_DIRECTORY "${MODULES_CONFIG_DIR}")

# Module discovery and validation
function(discover_modules)
    file(GLOB MODULE_DIRS RELATIVE "${MODULES_DIR}" "${MODULES_DIR}/mod-*")
    
    foreach(MODULE_DIR ${MODULE_DIRS})
        set(MODULE_PATH "${MODULES_DIR}/${MODULE_DIR}")
        set(MODULE_JSON "${MODULE_PATH}/module.json")
        set(MODULE_CMAKE "${MODULE_PATH}/CMakeLists.txt")
        
        # Check if module has required files
        if(EXISTS "${MODULE_JSON}" AND EXISTS "${MODULE_CMAKE}")
            message(STATUS "Found module: ${MODULE_DIR}")
            
            # Read module metadata
            file(READ "${MODULE_JSON}" MODULE_JSON_CONTENT)
            
            # Parse basic module info (simplified JSON parsing)
            string(REGEX MATCH "\"name\"[[:space:]]*:[[:space:]]*\"([^\"]+)\"" MODULE_NAME_MATCH "${MODULE_JSON_CONTENT}")
            if(MODULE_NAME_MATCH)
                set(MODULE_NAME "${CMAKE_MATCH_1}")
            else()
                set(MODULE_NAME "${MODULE_DIR}")
            endif()
            
            string(REGEX MATCH "\"version\"[[:space:]]*:[[:space:]]*\"([^\"]+)\"" MODULE_VERSION_MATCH "${MODULE_JSON_CONTENT}")
            if(MODULE_VERSION_MATCH)
                set(MODULE_VERSION "${CMAKE_MATCH_1}")
            else()
                set(MODULE_VERSION "1.0.0")
            endif()
            
            string(REGEX MATCH "\"enabled\"[[:space:]]*:[[:space:]]*(true|false)" MODULE_ENABLED_MATCH "${MODULE_JSON_CONTENT}")
            if(MODULE_ENABLED_MATCH)
                set(MODULE_ENABLED "${CMAKE_MATCH_1}")
            else()
                set(MODULE_ENABLED "true")
            endif()
            
            # Check if module should be built
            if(MODULE_ENABLED STREQUAL "true")
                message(STATUS "  - Name: ${MODULE_NAME}")
                message(STATUS "  - Version: ${MODULE_VERSION}")
                message(STATUS "  - Status: Enabled")
                
                # Add module to build
                add_subdirectory("${MODULE_DIR}")
                
                # Copy configuration files
                file(GLOB MODULE_CONFIGS "${MODULE_PATH}/conf/*.conf")
                foreach(CONFIG_FILE ${MODULE_CONFIGS})
                    get_filename_component(CONFIG_NAME "${CONFIG_FILE}" NAME)
                    configure_file("${CONFIG_FILE}" "${MODULES_CONFIG_DIR}/${CONFIG_NAME}" COPYONLY)
                endforeach()
                
                # Store module info for later use
                set_property(GLOBAL APPEND PROPERTY ENABLED_MODULES "${MODULE_DIR}")
                set_property(GLOBAL PROPERTY "MODULE_${MODULE_DIR}_NAME" "${MODULE_NAME}")
                set_property(GLOBAL PROPERTY "MODULE_${MODULE_DIR}_VERSION" "${MODULE_VERSION}")
            else()
                message(STATUS "  - Status: Disabled (skipping)")
            endif()
        else()
            message(WARNING "Module ${MODULE_DIR} is missing required files (module.json or CMakeLists.txt)")
        endif()
    endforeach()
endfunction()

# Module validation function
function(validate_module MODULE_DIR)
    set(MODULE_PATH "${MODULES_DIR}/${MODULE_DIR}")
    set(MODULE_JSON "${MODULE_PATH}/module.json")
    
    if(NOT EXISTS "${MODULE_JSON}")
        message(FATAL_ERROR "Module ${MODULE_DIR} is missing module.json")
    endif()
    
    # Validate JSON structure (basic validation)
    file(READ "${MODULE_JSON}" JSON_CONTENT)
    
    # Check for required fields
    if(NOT JSON_CONTENT MATCHES "\"name\"")
        message(FATAL_ERROR "Module ${MODULE_DIR}: module.json missing 'name' field")
    endif()
    
    if(NOT JSON_CONTENT MATCHES "\"version\"")
        message(FATAL_ERROR "Module ${MODULE_DIR}: module.json missing 'version' field")
    endif()
    
    if(NOT JSON_CONTENT MATCHES "\"core_version\"")
        message(WARNING "Module ${MODULE_DIR}: module.json missing 'core_version' field")
    endif()
endfunction()

# Module dependency resolution
function(resolve_module_dependencies)
    get_property(ENABLED_MODULES GLOBAL PROPERTY ENABLED_MODULES)
    
    foreach(MODULE_DIR ${ENABLED_MODULES})
        set(MODULE_JSON "${MODULES_DIR}/${MODULE_DIR}/module.json")
        file(read "${MODULE_JSON}" JSON_CONTENT)
        
        # Extract dependencies (simplified)
        string(REGEX MATCHALL "\"dependencies\"[[:space:]]*:[[:space:]]*\\[([^\\]]+)\\]" DEPS_MATCH "${JSON_CONTENT}")
        if(DEPS_MATCH)
            string(REGEX REPLACE "\"" "" DEPS_CLEAN "${CMAKE_MATCH_1}")
            string(REPLACE "," ";" DEPS_LIST "${DEPS_CLEAN}")
            
            foreach(DEP ${DEPS_LIST})
                string(STRIP "${DEP}" DEP_CLEAN)
                message(STATUS "Module ${MODULE_DIR} depends on: ${DEP_CLEAN}")
                
                # Check if dependency is available
                if(NOT "mod-${DEP_CLEAN}" IN_LIST ENABLED_MODULES)
                    message(WARNING "Module ${MODULE_DIR} dependency '${DEP_CLEAN}' not found or disabled")
                endif()
            endforeach()
        endif()
    endforeach()
endfunction()

# Generate module registry
function(generate_module_registry)
    get_property(ENABLED_MODULES GLOBAL PROPERTY ENABLED_MODULES)
    
    set(REGISTRY_FILE "${CMAKE_BINARY_DIR}/ModuleRegistry.h")
    
    file(WRITE "${REGISTRY_FILE}" 
        "/*\n"
        " * Auto-generated module registry\n"
        " * Do not edit this file manually\n"
        " */\n\n"
        "#ifndef MODULE_REGISTRY_H\n"
        "#define MODULE_REGISTRY_H\n\n"
        "#include <vector>\n"
        "#include <string>\n\n"
        "struct ModuleInfo {\n"
        "    std::string name;\n"
        "    std::string version;\n"
        "    std::string directory;\n"
        "};\n\n"
        "class ModuleRegistry {\n"
        "public:\n"
        "    static std::vector<ModuleInfo> GetAvailableModules() {\n"
        "        return {\n"
    )
    
    foreach(MODULE_DIR ${ENABLED_MODULES})
        get_property(MODULE_NAME GLOBAL PROPERTY "MODULE_${MODULE_DIR}_NAME")
        get_property(MODULE_VERSION GLOBAL PROPERTY "MODULE_${MODULE_DIR}_VERSION")
        
        file(APPEND "${REGISTRY_FILE}" 
            "            {\"${MODULE_NAME}\", \"${MODULE_VERSION}\", \"${MODULE_DIR}\"},\n"
        )
    endforeach()
    
    file(APPEND "${REGISTRY_FILE}" 
        "        };\n"
        "    }\n"
        "};\n\n"
        "#endif // MODULE_REGISTRY_H\n"
    )
    
    message(STATUS "Generated module registry: ${REGISTRY_FILE}")
endfunction()

# Module installation function
function(install_modules)
    get_property(ENABLED_MODULES GLOBAL PROPERTY ENABLED_MODULES)
    
    foreach(MODULE_DIR ${ENABLED_MODULES})
        get_property(MODULE_NAME GLOBAL PROPERTY "MODULE_${MODULE_DIR}_NAME")
        
        # Install module library
        install(TARGETS "${MODULE_NAME}" 
                DESTINATION "${CMAKE_INSTALL_LIBDIR}/modules"
                COMPONENT modules)
        
        # Install module configuration
        file(GLOB MODULE_CONFIGS "${MODULES_DIR}/${MODULE_DIR}/conf/*.conf")
        if(MODULE_CONFIGS)
            install(FILES ${MODULE_CONFIGS}
                    DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/modules"
                    COMPONENT modules)
        endif()
        
        # Install module documentation
        if(EXISTS "${MODULES_DIR}/${MODULE_DIR}/README.md")
            install(FILES "${MODULES_DIR}/${MODULE_DIR}/README.md"
                    DESTINATION "${CMAKE_INSTALL_DOCDIR}/modules/${MODULE_DIR}"
                    COMPONENT modules)
        endif()
    endforeach()
endfunction()

# Module testing function
function(setup_module_tests)
    if(BUILD_TESTING)
        get_property(ENABLED_MODULES GLOBAL PROPERTY ENABLED_MODULES)
        
        foreach(MODULE_DIR ${ENABLED_MODULES})
            set(TEST_DIR "${MODULES_DIR}/${MODULE_DIR}/tests")
            if(EXISTS "${TEST_DIR}")
                message(STATUS "Setting up tests for module: ${MODULE_DIR}")
                
                file(GLOB TEST_SOURCES "${TEST_DIR}/*.cpp")
                if(TEST_SOURCES)
                    get_property(MODULE_NAME GLOBAL PROPERTY "MODULE_${MODULE_DIR}_NAME")
                    
                    add_executable("test_${MODULE_NAME}" ${TEST_SOURCES})
                    target_link_libraries("test_${MODULE_NAME}" "${MODULE_NAME}" gtest gtest_main)
                    
                    add_test(NAME "${MODULE_NAME}_tests" COMMAND "test_${MODULE_NAME}")
                endif()
            endif()
        endforeach()
    endif()
endfunction()

# Development helper functions
function(create_module_template MODULE_NAME)
    set(TEMPLATE_DIR "${MODULES_DIR}/mod-${MODULE_NAME}")
    
    if(EXISTS "${TEMPLATE_DIR}")
        message(FATAL_ERROR "Module ${MODULE_NAME} already exists")
    endif()
    
    # Create directory structure
    file(MAKE_DIRECTORY "${TEMPLATE_DIR}")
    file(MAKE_DIRECTORY "${TEMPLATE_DIR}/src")
    file(MAKE_DIRECTORY "${TEMPLATE_DIR}/conf")
    file(MAKE_DIRECTORY "${TEMPLATE_DIR}/tests")
    
    # Create basic module.json
    file(WRITE "${TEMPLATE_DIR}/module.json" 
        "{\n"
        "  \"name\": \"${MODULE_NAME}\",\n"
        "  \"version\": \"1.0.0\",\n"
        "  \"description\": \"Description for ${MODULE_NAME} module\",\n"
        "  \"author\": \"Your Name\",\n"
        "  \"core_version\": \"3.3.5\",\n"
        "  \"enabled\": true,\n"
        "  \"dependencies\": [],\n"
        "  \"features\": []\n"
        "}\n"
    )
    
    message(STATUS "Created module template: ${TEMPLATE_DIR}")
endfunction()

# Main execution
message(STATUS "Configuring TrinityCore Modules...")

# Set module build options
option(BUILD_MODULES "Build modules" ON)
option(BUILD_MODULE_TESTS "Build module tests" OFF)
option(VALIDATE_MODULES "Validate module configurations" ON)
option(INSTALL_MODULES "Install modules" ON)

if(BUILD_MODULES)
    # Discover and configure modules
    discover_modules()
    
    # Validate modules if requested
    if(VALIDATE_MODULES)
        get_property(ENABLED_MODULES GLOBAL PROPERTY ENABLED_MODULES)
        foreach(MODULE_DIR ${ENABLED_MODULES})
            validate_module("${MODULE_DIR}")
        endforeach()
    endif()
    
    # Resolve dependencies
    resolve_module_dependencies()
    
    # Generate module registry
    generate_module_registry()
    
    # Setup tests
    if(BUILD_MODULE_TESTS)
        setup_module_tests()
    endif()
    
    # Setup installation
    if(INSTALL_MODULES)
        install_modules()
    endif()
    
    # Print summary
    get_property(ENABLED_MODULES GLOBAL PROPERTY ENABLED_MODULES)
    list(LENGTH ENABLED_MODULES MODULE_COUNT)
    message(STATUS "Configured ${MODULE_COUNT} modules successfully")
else()
    message(STATUS "Module building is disabled")
endif()

# Add custom targets for module management
add_custom_target(modules-clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${MODULES_OUTPUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${MODULES_OUTPUT_DIR}"
    COMMENT "Cleaning module build directory"
)

add_custom_target(modules-install
    COMMAND ${CMAKE_COMMAND} --build . --target install --component modules
    COMMENT "Installing modules"
)

add_custom_target(modules-list
    COMMAND ${CMAKE_COMMAND} -E echo "Available modules:"
    COMMENT "Listing available modules"
)

# Add module list generation
get_property(ENABLED_MODULES GLOBAL PROPERTY ENABLED_MODULES)
foreach(MODULE_DIR ${ENABLED_MODULES})
    get_property(MODULE_NAME GLOBAL PROPERTY "MODULE_${MODULE_DIR}_NAME")
    get_property(MODULE_VERSION GLOBAL PROPERTY "MODULE_${MODULE_DIR}_VERSION")
    
    add_custom_command(TARGET modules-list POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "  - ${MODULE_NAME} (${MODULE_VERSION}) [${MODULE_DIR}]"
        VERBATIM
    )
endforeach()

# Export module information for parent build system
set(MODULES_FOUND ${MODULE_COUNT} PARENT_SCOPE)
set(MODULES_LIST "${ENABLED_MODULES}" PARENT_SCOPE)

message(STATUS "Modules configuration complete")