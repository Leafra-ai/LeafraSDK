cmake_minimum_required(VERSION 3.18)
project(LeafraSDK VERSION 1.0.0 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform detection
if(APPLE)
    set(LEAFRA_PLATFORM "apple")
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        set(LEAFRA_PLATFORM_IOS ON)
    else()
        set(LEAFRA_PLATFORM_MACOS ON)
    endif()
elseif(ANDROID)
    set(LEAFRA_PLATFORM "android")
    set(LEAFRA_PLATFORM_ANDROID ON)
elseif(WIN32)
    set(LEAFRA_PLATFORM "windows")
    set(LEAFRA_PLATFORM_WINDOWS ON)
else()
    set(LEAFRA_PLATFORM "linux")
    set(LEAFRA_PLATFORM_LINUX ON)
endif()

# Build options
option(LEAFRA_BUILD_SHARED "Build shared library" ON)
option(LEAFRA_BUILD_TESTS "Build tests" OFF)
option(LEAFRA_BUILD_EXAMPLES "Build examples" OFF)
option(LEAFRA_BUILD_RN_BINDINGS "Build React Native bindings" ON)

# Compiler flags
if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/corecpp/include)

# Add subdirectories
add_subdirectory(corecpp)

# Platform-specific bindings
if(LEAFRA_BUILD_RN_BINDINGS)
    if(LEAFRA_PLATFORM_IOS OR LEAFRA_PLATFORM_MACOS)
        add_subdirectory(react-native/ios)
    endif()
    
    if(LEAFRA_PLATFORM_ANDROID)
        add_subdirectory(react-native/android)
    endif()
    
    if(LEAFRA_PLATFORM_WINDOWS)
        add_subdirectory(react-native/windows)
    endif()
endif()

# Export targets
export(TARGETS LeafraCore FILE LeafraSDKTargets.cmake)

# Install configuration
install(EXPORT LeafraSDKTargets
    FILE LeafraSDKTargets.cmake
    DESTINATION lib/cmake/LeafraSDK
)

# Package configuration
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/LeafraSDKConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/LeafraSDKConfig.cmake"
    INSTALL_DESTINATION lib/cmake/LeafraSDK
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/LeafraSDKConfig.cmake"
    DESTINATION lib/cmake/LeafraSDK
) 