# CMake script to list available modules

set(MODULES_DIR "${CMAKE_SOURCE_DIR}/modules")

if(NOT EXISTS ${MODULES_DIR})
    message("No modules directory found at: ${MODULES_DIR}")
    return()
endif()

# Find all module directories
file(GLOB MODULE_DIRS LIST_DIRECTORIES true "${MODULES_DIR}/mod-*")

if(NOT MODULE_DIRS)
    message("No modules found in: ${MODULES_DIR}")
    return()
endif()

message("Found ${CMAKE_MATCH_COUNT} modules:")
message("")

foreach(MODULE_DIR ${MODULE_DIRS})
    if(IS_DIRECTORY ${MODULE_DIR})
        get_filename_component(MODULE_NAME ${MODULE_DIR} NAME)
        
        # Check for module.json
        set(MODULE_JSON "${MODULE_DIR}/module.json")
        if(EXISTS ${MODULE_JSON})
            # Try to read basic info from module.json
            file(READ ${MODULE_JSON} MODULE_JSON_CONTENT)
            
            # Extract name and version (basic regex matching)
            string(REGEX MATCH "\"name\"[[:space:]]*:[[:space:]]*\"([^\"]+)\"" NAME_MATCH ${MODULE_JSON_CONTENT})
            string(REGEX MATCH "\"version\"[[:space:]]*:[[:space:]]*\"([^\"]+)\"" VERSION_MATCH ${MODULE_JSON_CONTENT})
            string(REGEX MATCH "\"description\"[[:space:]]*:[[:space:]]*\"([^\"]+)\"" DESC_MATCH ${MODULE_JSON_CONTENT})
            
            if(NAME_MATCH)
                set(DISPLAY_NAME ${CMAKE_MATCH_1})
            else()
                set(DISPLAY_NAME ${MODULE_NAME})
            endif()
            
            if(VERSION_MATCH)
                set(MODULE_VERSION ${CMAKE_MATCH_1})
            else()
                set(MODULE_VERSION "unknown")
            endif()
            
            if(DESC_MATCH)
                set(MODULE_DESC ${CMAKE_MATCH_1})
            else()
                set(MODULE_DESC "No description available")
            endif()
            
            message("  ${DISPLAY_NAME} (${MODULE_VERSION})")
            message("    Directory: ${MODULE_NAME}")
            message("    Description: ${MODULE_DESC}")
            
            # Check for source files
            file(GLOB_RECURSE MODULE_SOURCES "${MODULE_DIR}/src/*.cpp" "${MODULE_DIR}/src/*.h")
            list(LENGTH MODULE_SOURCES SOURCE_COUNT)
            message("    Source files: ${SOURCE_COUNT}")
            
            # Check for config files
            file(GLOB MODULE_CONFIGS "${MODULE_DIR}/conf/*.conf")
            list(LENGTH MODULE_CONFIGS CONFIG_COUNT)
            message("    Config files: ${CONFIG_COUNT}")
            
        else()
            message("  ${MODULE_NAME}")
            message("    Status: Missing module.json")
        endif()
        
        message("")
    endif()
endforeach()

message("Use 'cmake --build . --target validate-modules' to validate module configurations.")