cmake_minimum_required(VERSION 3.18)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project name
project(LeafraSDKCmdLineExample)

# Disable React Native bindings for command line build
set(LEAFRA_BUILD_RN_BINDINGS OFF)

# Find the LeafraCore library
# This assumes the SDK is built and installed, or we're building from the parent directory
if(NOT TARGET LeafraCore)
    # Try to find LeafraCore in parent directories
    find_path(LEAFRA_REPO_ROOT 
        NAMES sdk/corecpp/include/leafra/leafra_core.h
        PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../..
        NO_DEFAULT_PATH
    )
    
    if(LEAFRA_REPO_ROOT)
        message(STATUS "Found LeafraSDK repository at: ${LEAFRA_REPO_ROOT}")
        # Include the full SDK directory to get PDFium configuration
        add_subdirectory(${LEAFRA_REPO_ROOT}/sdk leafra-sdk)
    else()
        message(FATAL_ERROR "Could not find LeafraSDK. Please ensure this example is in the correct directory structure.")
    endif()
endif()

# Create the executable
add_executable(sdkcmdline sdkcmdline.cpp)

# Link against LeafraCore
target_link_libraries(sdkcmdline PRIVATE LeafraCore)

# Ensure we have the same compile definitions as LeafraCore
target_compile_definitions(sdkcmdline PRIVATE LEAFRA_HAS_FAISS=1)

# Include directories - point to the SDK headers
if(LEAFRA_REPO_ROOT)
    target_include_directories(sdkcmdline PRIVATE ${LEAFRA_REPO_ROOT}/sdk/corecpp/include)
    # Add SDK root macros for use in C++ code
    target_compile_definitions(sdkcmdline PRIVATE
        LEAFRA_SDK_ROOT="${LEAFRA_REPO_ROOT}/sdk"
        LEAFRA_SDK_MODELS_ROOT="${LEAFRA_REPO_ROOT}/sdk/corecpp/third_party/models"
    )
else()
    # Fallback - assume we're building as part of the main project
    target_include_directories(sdkcmdline PRIVATE ../../sdk/corecpp/include)
    target_compile_definitions(sdkcmdline PRIVATE
        LEAFRA_SDK_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../sdk"
        LEAFRA_SDK_MODELS_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/../../sdk/corecpp/third_party/models"
    )
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

# Configuration summary
message(STATUS "✅ LeafraSDK Command Line Example configured successfully")
message(STATUS "📱 Supported platforms: macOS, Linux, Windows (Desktop only)") 