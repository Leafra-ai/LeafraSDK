cmake_minimum_required(VERSION 3.20)
project(LeafraSQLiteTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform detection
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(PLATFORM_IOS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(PLATFORM_MACOS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PLATFORM_WINDOWS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PLATFORM_LINUX TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(PLATFORM_ANDROID TRUE)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../src
)

# Find required packages
if(PLATFORM_MACOS OR PLATFORM_IOS)
    # Use system SQLite on Apple platforms
    # Link directly with -lsqlite3 instead of using find_library
    
    # Apple frameworks for file manager
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
    find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation REQUIRED)
    
    set(PLATFORM_LIBRARIES 
        "-lsqlite3"
        ${FOUNDATION_FRAMEWORK}
        ${CORE_FOUNDATION_FRAMEWORK}
    )
    
    # Define that SQLite is available
    add_definitions(-DLEAFRA_HAS_SQLITE)
    
    # Enable Objective-C++ for Apple platforms (removed for now to debug)
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -x objective-c++")
    
elseif(PLATFORM_WINDOWS)
    # Windows-specific SQLite setup
    # TODO: Add Windows SQLite library configuration
    set(PLATFORM_LIBRARIES "")
    
elseif(PLATFORM_LINUX)
    # Linux-specific SQLite setup
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SQLITE3 REQUIRED sqlite3)
    
    set(PLATFORM_LIBRARIES ${SQLITE3_LIBRARIES})
    include_directories(${SQLITE3_INCLUDE_DIRS})
    
elseif(PLATFORM_ANDROID)
    # Android-specific SQLite setup
    # TODO: Add Android SQLite library configuration
    set(PLATFORM_LIBRARIES "")
endif()

# Source files for the main library components we're testing
set(LEAFRA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/leafra_sqlite.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/logger.cpp
)

# File manager needs to be compiled as Objective-C++ on Apple platforms
if(PLATFORM_MACOS OR PLATFORM_IOS)
    set(LEAFRA_OBJCXX_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/leafra_filemanager.cpp
    )
else()
    set(LEAFRA_SOURCES ${LEAFRA_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/leafra_filemanager.cpp
    )
endif()

# Test executables
set(TEST_TARGETS
    test_sqlite_basic
    test_sqlite_advanced
)

# Create test executables
foreach(TARGET ${TEST_TARGETS})
    add_executable(${TARGET}
        ${TARGET}.cpp
        ${LEAFRA_SOURCES}
    )
    
    # Add Objective-C++ sources for Apple platforms
    if(PLATFORM_MACOS OR PLATFORM_IOS)
        target_sources(${TARGET} PRIVATE ${LEAFRA_OBJCXX_SOURCES})
        set_source_files_properties(${LEAFRA_OBJCXX_SOURCES} PROPERTIES
            COMPILE_FLAGS "-x objective-c++"
        )
    endif()
    
    # Add include directories
    target_include_directories(${TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../include
    )
    
    # Link libraries
    target_link_libraries(${TARGET}
        ${PLATFORM_LIBRARIES}
    )
    
    # Platform-specific configurations
    if(PLATFORM_IOS)
        # iOS-specific settings
        set_target_properties(${TARGET} PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "13.0"
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.leafra.sqlite.${TARGET}"
        )
        
    elseif(PLATFORM_MACOS)
        # macOS-specific settings
        set_target_properties(${TARGET} PROPERTIES
            MACOSX_DEPLOYMENT_TARGET "10.15"
        )
        
    elseif(PLATFORM_WINDOWS)
        # Windows-specific settings
        set_target_properties(${TARGET} PROPERTIES
            WIN32_EXECUTABLE TRUE
        )
        
    elseif(PLATFORM_LINUX)
        # Linux-specific settings
        target_compile_options(${TARGET} PRIVATE ${SQLITE3_CFLAGS_OTHER})
        
    elseif(PLATFORM_ANDROID)
        # Android-specific settings
        set_target_properties(${TARGET} PROPERTIES
            ANDROID_STL "c++_shared"
        )
    endif()
    
    # Compiler definitions
    target_compile_definitions(${TARGET} PRIVATE
        LEAFRA_SQLITE_TESTS=1
    )
    
    # Platform-specific compile definitions
    if(PLATFORM_MACOS OR PLATFORM_IOS)
        target_compile_definitions(${TARGET} PRIVATE
            LEAFRA_USE_SYSTEM_SQLITE_HEADERS=1
        )
    endif()
    
    # Enable testing for this target
    add_test(NAME ${TARGET} COMMAND ${TARGET})
endforeach()

# Custom targets for different platforms

# macOS native build
add_custom_target(build_macos
    COMMAND ${CMAKE_COMMAND} -B build_macos -DCMAKE_BUILD_TYPE=Debug
    COMMAND ${CMAKE_COMMAND} --build build_macos
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building SQLite tests for macOS"
)

# iOS Simulator build
add_custom_target(build_ios_simulator
    COMMAND ${CMAKE_COMMAND} -B build_ios_simulator 
        -DCMAKE_SYSTEM_NAME=iOS 
        -DCMAKE_OSX_ARCHITECTURES=x86_64$<SEMICOLON>arm64
        -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0
        -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=""
        -G Xcode
    COMMAND xcodebuild -project build_ios_simulator/LeafraSQLiteTests.xcodeproj 
        -scheme test_sqlite_basic 
        -configuration Debug 
        -destination 'platform=iOS Simulator,name=iPhone 16 Pro' 
        build
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building SQLite tests for iOS Simulator"
)

# iOS Device build
add_custom_target(build_ios_device
    COMMAND ${CMAKE_COMMAND} -B build_ios_device
        -DCMAKE_SYSTEM_NAME=iOS
        -DCMAKE_OSX_ARCHITECTURES=arm64
        -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0
        -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM="${IOS_DEVELOPMENT_TEAM}"
        -G Xcode
    COMMAND xcodebuild -project build_ios_device/LeafraSQLiteTests.xcodeproj
        -scheme test_sqlite_basic
        -configuration Debug
        -destination 'generic/platform=iOS'
        build
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building SQLite tests for iOS Device"
)

# Windows build (placeholder)
add_custom_target(build_windows
    COMMAND echo "Windows build not yet implemented"
    COMMENT "Building SQLite tests for Windows (TODO)"
)

# Linux build (placeholder)
add_custom_target(build_linux
    COMMAND echo "Linux build not yet implemented"
    COMMENT "Building SQLite tests for Linux (TODO)"
)

# Android build (placeholder)
add_custom_target(build_android
    COMMAND echo "Android build not yet implemented"
    COMMENT "Building SQLite tests for Android (TODO)"
)

# Test running targets

# Run macOS tests
add_custom_target(test_macos
    COMMAND ./build_macos/test_sqlite_basic
    COMMAND ./build_macos/test_sqlite_advanced
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS build_macos
    COMMENT "Running SQLite tests on macOS"
)

# Run iOS Simulator tests
add_custom_target(test_ios_simulator
    COMMAND echo "Running SQLite tests on iOS Simulator..."
    COMMAND xcrun simctl spawn booted ./build_ios_simulator/Debug-iphonesimulator/test_sqlite_basic.app/test_sqlite_basic || echo "Note: Requires running iOS Simulator"
    COMMAND xcrun simctl spawn booted ./build_ios_simulator/Debug-iphonesimulator/test_sqlite_advanced.app/test_sqlite_advanced || echo "Note: Requires running iOS Simulator"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS build_ios_simulator
    COMMENT "Running SQLite tests on iOS Simulator"
)

# Enable CTest
enable_testing()

# Print configuration summary
message(STATUS "LeafraSDK SQLite Tests Configuration:")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
if(PLATFORM_MACOS OR PLATFORM_IOS)
    message(STATUS "  SQLite Library: ${SQLITE3_LIBRARY}")
    message(STATUS "  Foundation Framework: ${FOUNDATION_FRAMEWORK}")
endif()
message(STATUS "  Test Targets: ${TEST_TARGETS}") 