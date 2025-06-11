cmake_minimum_required(VERSION 3.18)

# Unit tests for CoreMLModel
project(CoreMLModelTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check for DEBUG flag and set build configuration
if(DEFINED ENV{DEBUG} AND "$ENV{DEBUG}" STREQUAL "1")
    message(STATUS "🐛 DEBUG mode enabled")
    set(CMAKE_BUILD_TYPE Debug)
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG=1")
    set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG=1")
    # Additional debug flags for better debugging experience
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fno-optimize-sibling-calls")
else()
    message(STATUS "🚀 Release mode (use DEBUG=1 for debug build)")
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Include directories
include_directories(../../../include)

# Check if we're on macOS/iOS for CoreML support
if(APPLE)
    # Find CoreML framework
    find_library(COREML_FRAMEWORK CoreML)
    find_library(FOUNDATION_FRAMEWORK Foundation)
    
    if(COREML_FRAMEWORK AND FOUNDATION_FRAMEWORK)
        message(STATUS "✅ Found CoreML framework: ${COREML_FRAMEWORK}")
        set(COREML_FOUND TRUE)
    else()
        message(WARNING "❌ CoreML framework not found")
        set(COREML_FOUND FALSE)
    endif()
else()
    message(WARNING "❌ CoreML tests require macOS/iOS platform")
    set(COREML_FOUND FALSE)
endif()

# Only build CoreML tests if CoreML is available
if(COREML_FOUND)
    # Common source files for CoreML tests
    set(COREML_SOURCES
        ../../../src/leafra_coreml.mm
        ../../../src/logger.cpp
    )

    # Main CoreML test executable
    add_executable(test_coreml_model 
        test_coreml_model.cpp
        ${COREML_SOURCES}
    )

    # Link with required frameworks
    target_link_libraries(test_coreml_model
        ${COREML_FRAMEWORK}
        ${FOUNDATION_FRAMEWORK}
    )

    # Add compile definitions
    target_compile_definitions(test_coreml_model PRIVATE 
        LEAFRA_HAS_COREML=1
    )

    # Debug-specific settings
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "🔧 Adding debug compile options")
        target_compile_options(test_coreml_model PRIVATE
            -fsanitize=address  # AddressSanitizer for memory error detection
            -fno-omit-frame-pointer
            -g3                 # Maximum debug info
        )
        target_link_options(test_coreml_model PRIVATE
            -fsanitize=address
        )
        target_compile_definitions(test_coreml_model PRIVATE
            DEBUG=1
            _DEBUG=1
        )
    endif()

    # Set Objective-C++ compilation for .mm files
    set_source_files_properties(../../../src/leafra_coreml.mm PROPERTIES 
        COMPILE_FLAGS "-x objective-c++")

    # Custom target to run CoreML tests
    add_custom_target(run_coreml_tests
        COMMAND test_coreml_model
        DEPENDS test_coreml_model
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running CoreML Model unit tests"
    )

    # Debug information
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "✅ CoreML debug tests configured successfully")
        message(STATUS "   - Debug symbols: enabled (-g3)")
        message(STATUS "   - Optimizations: disabled (-O0)")
        message(STATUS "   - AddressSanitizer: enabled")
        message(STATUS "   - Frame pointers: preserved")
        message(STATUS "   - Use 'make run_coreml_tests' to run tests")
        message(STATUS "   - Use 'lldb test_coreml_model' for debugging")
    else()
        message(STATUS "✅ CoreML tests configured successfully")
        message(STATUS "   - Use 'make run_coreml_tests' to run tests")
        message(STATUS "   - Use 'DEBUG=1 cmake ..' for debug build")
    endif()
else()
    message(STATUS "⚠️  CoreML tests skipped (CoreML framework not available)")
endif() 