cmake_minimum_required(VERSION 3.18)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project name
project(LeafraSDKCmdLineExample)

# Find the LeafraCore library
# This assumes the SDK is built and installed, or we're building from the parent directory
if(NOT TARGET LeafraCore)
    # Try to find LeafraCore in parent directories
    find_path(LEAFRA_SDK_ROOT 
        NAMES sdk/corecpp/include/leafra/leafra_core.h
        PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../..
        NO_DEFAULT_PATH
    )
    
    if(LEAFRA_SDK_ROOT)
        message(STATUS "Found LeafraSDK at: ${LEAFRA_SDK_ROOT}")
        add_subdirectory(${LEAFRA_SDK_ROOT}/sdk/corecpp LeafraCore)
    else()
        message(FATAL_ERROR "LeafraCore library not found. Please build the SDK first or ensure it's in the parent directory.")
    endif()
endif()

# SDK Command Line Application (Example)
add_executable(sdkcmdline sdkcmdline.cpp)

# Link with LeafraCore
target_link_libraries(sdkcmdline LeafraCore)

# Include directories - point to the SDK headers
if(LEAFRA_SDK_ROOT)
    target_include_directories(sdkcmdline PRIVATE ${LEAFRA_SDK_ROOT}/sdk/corecpp/include)
else()
    # Fallback - assume we're building as part of the main project
    target_include_directories(sdkcmdline PRIVATE ../../sdk/corecpp/include)
endif()

# Platform-specific settings
if(APPLE)
    message(STATUS "✅ Building sdkcmdline for macOS")
elseif(UNIX AND NOT APPLE)
    message(STATUS "✅ Building sdkcmdline for Linux")
elseif(WIN32)
    message(STATUS "✅ Building sdkcmdline for Windows")
endif()

# Set output directory
set_target_properties(sdkcmdline PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

message(STATUS "✅ LeafraSDK Command Line Example configured successfully")
message(STATUS "📱 Supported platforms: macOS, Linux, Windows (Desktop only)") 