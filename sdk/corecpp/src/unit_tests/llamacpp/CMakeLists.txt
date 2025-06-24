cmake_minimum_required(VERSION 3.14)

# Unit tests for LlamaCppModel
project(LlamaCppModelTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(../../../include)

# Path to the .xcframework
set(XCFRAMEWORK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/prebuilt/llamacpp/apple/llama.xcframework")

# Detect platform and set appropriate headers path
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    if(CMAKE_OSX_SYSROOT MATCHES ".*simulator.*")
        set(XCFRAMEWORK_PLATFORM "ios-arm64_x86_64-simulator")
    else()
        set(XCFRAMEWORK_PLATFORM "ios-arm64")
    endif()
else()
    # Default to macOS
    set(XCFRAMEWORK_PLATFORM "macos-arm64_x86_64")
endif()

# Set headers path from xcframework
set(LLAMACPP_HEADERS_PATH "${XCFRAMEWORK_PATH}/${XCFRAMEWORK_PLATFORM}/llama.framework/Headers")
include_directories(${LLAMACPP_HEADERS_PATH})

# Common source files for tests
set(COMMON_SOURCES
    ../../../src/logger.cpp
)

# Platform-specific sources
if(APPLE)
    set(PLATFORM_SOURCES
        ../../../src/leafra_llamacpp.cpp
    )
    # Set compile flags for LlamaCpp source
    set_source_files_properties(../../../src/leafra_llamacpp.cpp PROPERTIES
        COMPILE_FLAGS "-DLEAFRA_HAS_LLAMACPP=1"
    )
else()
    set(PLATFORM_SOURCES
        ../../../src/leafra_llamacpp.cpp
    )
    # Add the LEAFRA_HAS_LLAMACPP define for non-Apple platforms too
    set_source_files_properties(../../../src/leafra_llamacpp.cpp PROPERTIES
        COMPILE_FLAGS "-DLEAFRA_HAS_LLAMACPP=1"
    )
endif()

set(ALL_SOURCES ${COMMON_SOURCES} ${PLATFORM_SOURCES})

# Add individual test executables
add_executable(test_llamacpp_basic 
    test_llamacpp_simple.cpp
    ${ALL_SOURCES}
)

# Add compile definitions for the test executable
target_compile_definitions(test_llamacpp_basic PRIVATE LEAFRA_HAS_LLAMACPP=1)

# Platform-specific linking
if(APPLE)
    # Full path to the actual framework inside the .xcframework
    set(LLAMA_FRAMEWORK_DIR "${XCFRAMEWORK_PATH}/${XCFRAMEWORK_PLATFORM}")
    
    # Create imported framework target for proper CMake handling
    add_library(llama_framework SHARED IMPORTED)
    set_target_properties(llama_framework PROPERTIES
        IMPORTED_LOCATION "${LLAMA_FRAMEWORK_DIR}/llama.framework/llama"
        INTERFACE_INCLUDE_DIRECTORIES "${LLAMA_FRAMEWORK_DIR}/llama.framework/Headers"
        FRAMEWORK TRUE
        IMPORTED_IMPLIB "${LLAMA_FRAMEWORK_DIR}/llama.framework/llama"
    )
    
    # Set framework search paths and runtime paths via target properties
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        # iOS: Use @executable_path/Frameworks for app bundle
        set_target_properties(test_llamacpp_basic PROPERTIES
            XCODE_ATTRIBUTE_FRAMEWORK_SEARCH_PATHS "${LLAMA_FRAMEWORK_DIR}"
            XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/Frameworks"
            BUILD_WITH_INSTALL_RPATH TRUE
            INSTALL_RPATH "@executable_path/Frameworks"
        )
    else()
        # macOS: Use direct path to xcframework
        set_target_properties(test_llamacpp_basic PROPERTIES
            XCODE_ATTRIBUTE_FRAMEWORK_SEARCH_PATHS "${LLAMA_FRAMEWORK_DIR}"
            BUILD_WITH_INSTALL_RPATH TRUE
            INSTALL_RPATH "${LLAMA_FRAMEWORK_DIR}"
        )
    endif()
    
    # Link with the required Apple frameworks and imported llama framework
    target_link_libraries(test_llamacpp_basic 
        "-framework Foundation"
        "-framework CoreFoundation"
        llama_framework
    )
    
    # Set deployment targets for iOS compatibility
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
                # Define model file path
        set(MODEL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/models/llm/unsloth/Llama-3.2-3B-Instruct-Q4_K_M.gguf")
        
        # Check if model file exists
        if(NOT EXISTS ${MODEL_FILE})
            message(FATAL_ERROR "Model file not found: ${MODEL_FILE}")
        endif()
        
        # For iOS, we need Objective-C++ to use NSBundle APIs for bundle path detection
        enable_language(OBJCXX)
        set_source_files_properties(test_llamacpp_simple.cpp PROPERTIES
            COMPILE_FLAGS "-x objective-c++"
        )
        
        
        # Configure test target as iOS app bundle with Info.plist and model resource
        set_target_properties(test_llamacpp_basic PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "13.0"
            MACOSX_BUNDLE TRUE  # Note: MACOSX_BUNDLE works for both macOS and iOS (legacy CMake naming)
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.leafra.llamacpp.test_llamacpp_basic"
            RESOURCE "${MODEL_FILE}"
        )
        
        # Add model file as a resource to be bundled with the app
        target_sources(test_llamacpp_basic PRIVATE ${MODEL_FILE})
        set_source_files_properties(${MODEL_FILE} PROPERTIES
            MACOSX_PACKAGE_LOCATION Resources  # Note: Works for both macOS and iOS (legacy CMake naming)
        )
        
        # For iOS app bundles, we still need to manually embed the framework
        # since CMake's XCODE_EMBED_FRAMEWORKS doesn't work well with imported xcframeworks
        add_custom_command(TARGET test_llamacpp_basic POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory 
                $<TARGET_BUNDLE_DIR:test_llamacpp_basic>/Frameworks
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${LLAMA_FRAMEWORK_DIR}/llama.framework"
                $<TARGET_BUNDLE_DIR:test_llamacpp_basic>/Frameworks/llama.framework
            COMMAND codesign --force --sign - --timestamp=none 
                $<TARGET_BUNDLE_DIR:test_llamacpp_basic>/Frameworks/llama.framework
            COMMENT "Embedding and signing llama.framework in app bundle"
        )
    endif()
endif()

# Enable testing
enable_testing()

# Add tests to CTest
add_test(NAME LlamaCppBasic COMMAND test_llamacpp_basic) 