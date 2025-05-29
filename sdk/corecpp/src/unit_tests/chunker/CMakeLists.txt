cmake_minimum_required(VERSION 3.18)

# Unit tests for LeafraChunker
project(LeafraChunkerTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories (now we're deeper, need to go up more levels)
include_directories(../../../include)

# Source files
set(TEST_SOURCES
    test_leafra_chunker.cpp
    ../../leafra_chunker.cpp
)

# Create test executable
add_executable(leafra_chunker_tests ${TEST_SOURCES})

# Create additional demonstration/example executables
add_executable(test_token_chunking 
    test_token_chunking.cpp
    ../../leafra_chunker.cpp
)

add_executable(test_unified_api
    test_unified_api.cpp
    ../../leafra_chunker.cpp
)

# Platform-specific settings
if(APPLE)
    # macOS/iOS specific settings
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        set_target_properties(leafra_chunker_tests PROPERTIES
            XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
            XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ""
        )
    endif()
elseif(WIN32)
    # Windows specific settings
    if(MSVC)
        target_compile_options(leafra_chunker_tests PRIVATE /W4)
    endif()
elseif(ANDROID)
    # Android specific settings
    target_link_libraries(leafra_chunker_tests log)
elseif(UNIX)
    # Linux specific settings
    target_compile_options(leafra_chunker_tests PRIVATE -Wall -Wextra)
endif()

# Compiler definitions for cross-platform compatibility
target_compile_definitions(leafra_chunker_tests PRIVATE
    $<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS>
    $<$<PLATFORM_ID:Android>:__ANDROID__>
)

# Custom target to run tests
add_custom_target(run_chunker_tests
    COMMAND leafra_chunker_tests
    DEPENDS leafra_chunker_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running LeafraChunker unit tests"
)

# Custom targets for demonstration executables
add_custom_target(run_token_demo
    COMMAND test_token_chunking
    DEPENDS test_token_chunking
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running token chunking demonstration"
)

add_custom_target(run_unified_demo
    COMMAND test_unified_api
    DEPENDS test_unified_api
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running unified API demonstration"
)

# Install target (optional)
install(TARGETS leafra_chunker_tests test_token_chunking test_unified_api
    RUNTIME DESTINATION bin/tests
) 