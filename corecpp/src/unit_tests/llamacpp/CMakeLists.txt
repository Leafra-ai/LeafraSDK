cmake_minimum_required(VERSION 3.18)

# Unit tests for LlamaCpp
project(LlamaCppModelTests)

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
include_directories(../../include)

# Check if LlamaCpp is available
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../include/leafra/leafra_llamacpp.h")
    message(STATUS "✅ Found LlamaCpp header")
    set(LLAMACPP_FOUND TRUE)
else()
    message(WARNING "❌ LlamaCpp header not found")
    set(LLAMACPP_FOUND FALSE)
endif()

# Only build LlamaCpp tests if LlamaCpp is available
if(LLAMACPP_FOUND)
    # Common source files for LlamaCpp tests
    set(LLAMACPP_SOURCES
        ../../src/leafra_llamacpp.cpp
        ../../src/logger.cpp
    )

    # Simple test executable (without Google Test)
    add_executable(test_llamacpp_simple
        test_llamacpp_simple.cpp
        ${LLAMACPP_SOURCES}
    )

    # Link with required libraries
    target_link_libraries(test_llamacpp_simple
        pthread
    )

    # Add compile definitions
    target_compile_definitions(test_llamacpp_simple PRIVATE 
        LEAFRA_HAS_LLAMACPP=1
    )

    # Debug-specific settings
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "🔧 Adding debug compile options")
        target_compile_options(test_llamacpp_simple PRIVATE
            -fsanitize=address  # AddressSanitizer for memory error detection
            -fno-omit-frame-pointer
            -g3                 # Maximum debug info
        )
        target_link_options(test_llamacpp_simple PRIVATE
            -fsanitize=address
        )
        target_compile_definitions(test_llamacpp_simple PRIVATE
            DEBUG=1
            _DEBUG=1
        )
    endif()

    # Custom target to run LlamaCpp tests
    add_custom_target(run_llamacpp_tests
        COMMAND test_llamacpp_simple
        DEPENDS test_llamacpp_simple
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running LlamaCpp Model unit tests"
    )

    # Debug information
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "✅ LlamaCpp debug tests configured successfully")
        message(STATUS "   - Debug symbols: enabled (-g3)")
        message(STATUS "   - Optimizations: disabled (-O0)")
        message(STATUS "   - AddressSanitizer: enabled")
        message(STATUS "   - Frame pointers: preserved")
        message(STATUS "   - Use 'make run_llamacpp_tests' to run tests")
        message(STATUS "   - Use 'lldb test_llamacpp_simple' for debugging")
    else()
        message(STATUS "✅ LlamaCpp tests configured successfully")
        message(STATUS "   - Use 'make run_llamacpp_tests' to run tests")
        message(STATUS "   - Use 'DEBUG=1 cmake ..' for debug build")
    endif()
else()
    message(STATUS "⚠️  LlamaCpp tests skipped (LlamaCpp not available)")
endif() 