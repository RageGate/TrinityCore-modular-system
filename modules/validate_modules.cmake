# CMake script to validate module configurations

set(MODULES_DIR "${CMAKE_SOURCE_DIR}/modules")
set(VALIDATION_ERRORS 0)
set(VALIDATION_WARNINGS 0)

function(log_error MESSAGE)
    message("ERROR: ${MESSAGE}")
    math(EXPR VALIDATION_ERRORS "${VALIDATION_ERRORS} + 1")
    set(VALIDATION_ERRORS ${VALIDATION_ERRORS} PARENT_SCOPE)
endfunction()

function(log_warning MESSAGE)
    message("WARNING: ${MESSAGE}")
    math(EXPR VALIDATION_WARNINGS "${VALIDATION_WARNINGS} + 1")
    set(VALIDATION_WARNINGS ${VALIDATION_WARNINGS} PARENT_SCOPE)
endfunction()

function(log_info MESSAGE)
    message("INFO: ${MESSAGE}")
endfunction()

function(validate_json_file FILEPATH)
    if(NOT EXISTS ${FILEPATH})
        log_error("Missing required file: ${FILEPATH}")
        return()
    endif()
    
    file(READ ${FILEPATH} JSON_CONTENT)
    
    # Basic JSON validation (check for required fields)
    if(NOT JSON_CONTENT MATCHES "\"name\"")
        log_error("${FILEPATH}: Missing 'name' field")
    endif()
    
    if(NOT JSON_CONTENT MATCHES "\"version\"")
        log_error("${FILEPATH}: Missing 'version' field")
    endif()
    
    if(NOT JSON_CONTENT MATCHES "\"description\"")
        log_warning("${FILEPATH}: Missing 'description' field")
    endif()
    
    if(NOT JSON_CONTENT MATCHES "\"author\"")
        log_warning("${FILEPATH}: Missing 'author' field")
    endif()
endfunction()

function(validate_cmake_file FILEPATH)
    if(NOT EXISTS ${FILEPATH})
        log_error("Missing required file: ${FILEPATH}")
        return()
    endif()
    
    file(READ ${FILEPATH} CMAKE_CONTENT)
    
    # Check for basic CMake structure
    if(NOT CMAKE_CONTENT MATCHES "cmake_minimum_required")
        log_warning("${FILEPATH}: Missing cmake_minimum_required")
    endif()
    
    if(NOT CMAKE_CONTENT MATCHES "add_library|add_executable")
        log_warning("${FILEPATH}: No library or executable targets found")
    endif()
endfunction()

function(validate_source_files MODULE_DIR)
    file(GLOB_RECURSE CPP_FILES "${MODULE_DIR}/src/*.cpp")
    file(GLOB_RECURSE H_FILES "${MODULE_DIR}/src/*.h" "${MODULE_DIR}/src/*.hpp")
    
    list(LENGTH CPP_FILES CPP_COUNT)
    list(LENGTH H_FILES H_COUNT)
    
    if(CPP_COUNT EQUAL 0 AND H_COUNT EQUAL 0)
        log_warning("${MODULE_DIR}: No source files found in src/ directory")
    endif()
    
    # Check for main module files
    get_filename_component(MODULE_NAME ${MODULE_DIR} NAME)
    string(REGEX REPLACE "^mod-" "" CLEAN_NAME ${MODULE_NAME})
    string(SUBSTRING ${CLEAN_NAME} 0 1 FIRST_CHAR)
    string(TOUPPER ${FIRST_CHAR} FIRST_CHAR_UPPER)
    string(SUBSTRING ${CLEAN_NAME} 1 -1 REST_NAME)
    set(EXPECTED_CLASS "${FIRST_CHAR_UPPER}${REST_NAME}Module")
    
    set(MAIN_HEADER "${MODULE_DIR}/src/${EXPECTED_CLASS}.h")
    set(MAIN_SOURCE "${MODULE_DIR}/src/${EXPECTED_CLASS}.cpp")
    
    if(NOT EXISTS ${MAIN_HEADER})
        log_warning("${MODULE_DIR}: Expected main header file not found: ${EXPECTED_CLASS}.h")
    endif()
    
    if(NOT EXISTS ${MAIN_SOURCE})
        log_warning("${MODULE_DIR}: Expected main source file not found: ${EXPECTED_CLASS}.cpp")
    endif()
endfunction()

function(validate_config_files MODULE_DIR)
    file(GLOB CONF_FILES "${MODULE_DIR}/conf/*.conf")
    
    list(LENGTH CONF_FILES CONF_COUNT)
    if(CONF_COUNT EQUAL 0)
        log_warning("${MODULE_DIR}: No configuration files found in conf/ directory")
    endif()
endfunction()

function(validate_module MODULE_DIR)
    get_filename_component(MODULE_NAME ${MODULE_DIR} NAME)
    log_info("Validating module: ${MODULE_NAME}")
    
    # Validate directory structure
    if(NOT IS_DIRECTORY "${MODULE_DIR}/src")
        log_error("${MODULE_DIR}: Missing src/ directory")
    endif()
    
    if(NOT IS_DIRECTORY "${MODULE_DIR}/conf")
        log_warning("${MODULE_DIR}: Missing conf/ directory")
    endif()
    
    # Validate required files
    validate_json_file("${MODULE_DIR}/module.json")
    validate_cmake_file("${MODULE_DIR}/CMakeLists.txt")
    
    # Validate source files
    validate_source_files(${MODULE_DIR})
    
    # Validate config files
    validate_config_files(${MODULE_DIR})
    
    # Check for README
    if(NOT EXISTS "${MODULE_DIR}/README.md")
        log_warning("${MODULE_DIR}: Missing README.md file")
    endif()
endfunction()

# Main validation logic
message("Validating modules in: ${MODULES_DIR}")
message("")

if(NOT EXISTS ${MODULES_DIR})
    log_error("Modules directory not found: ${MODULES_DIR}")
    return()
endif()

# Find all module directories
file(GLOB MODULE_DIRS LIST_DIRECTORIES true "${MODULES_DIR}/mod-*")

if(NOT MODULE_DIRS)
    log_info("No modules found in: ${MODULES_DIR}")
    return()
endif()

# Validate each module
foreach(MODULE_DIR ${MODULE_DIRS})
    if(IS_DIRECTORY ${MODULE_DIR})
        validate_module(${MODULE_DIR})
        message("")
    endif()
endforeach()

# Summary
message("Validation Summary:")
message("  Errors: ${VALIDATION_ERRORS}")
message("  Warnings: ${VALIDATION_WARNINGS}")

if(VALIDATION_ERRORS GREATER 0)
    message("")
    message("Validation FAILED with ${VALIDATION_ERRORS} errors.")
    message("Please fix the errors before building.")
else()
    message("")
    message("Validation PASSED.")
    if(VALIDATION_WARNINGS GREATER 0)
        message("Consider addressing the ${VALIDATION_WARNINGS} warnings for better module quality.")
    endif()
endif()