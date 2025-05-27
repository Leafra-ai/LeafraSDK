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

# PDFium Integration
set(PDFIUM_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/corecpp/third_party/prebuilt/pdfium/paulocoutinhox")

# Platform-specific PDFium configuration
if(APPLE)
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        # iOS - Use XCFramework
        set(PDFIUM_XCFRAMEWORK_PATH "${PDFIUM_ROOT_DIR}/ios/release/pdfium.xcframework")
        
        # Determine iOS platform (device vs simulator)
        if(CMAKE_OSX_SYSROOT MATCHES "iPhoneSimulator")
            set(PDFIUM_PLATFORM_DIR "${PDFIUM_XCFRAMEWORK_PATH}/ios-arm64_x86_64-simulator")
        else()
            set(PDFIUM_PLATFORM_DIR "${PDFIUM_XCFRAMEWORK_PATH}/ios-arm64")
        endif()
        
        set(PDFIUM_LIBRARY "${PDFIUM_PLATFORM_DIR}/libpdfium.a")
        set(PDFIUM_INCLUDE_DIR "${PDFIUM_ROOT_DIR}/ios/release/include")
        
    else()
        # macOS
        set(PDFIUM_LIBRARY "${PDFIUM_ROOT_DIR}/macos/release/lib/libpdfium.a")
        set(PDFIUM_INCLUDE_DIR "${PDFIUM_ROOT_DIR}/macos/release/include")
    endif()
    
elseif(ANDROID)
    # Android - Select appropriate ABI
    if(ANDROID_ABI STREQUAL "arm64-v8a")
        set(PDFIUM_LIBRARY "${PDFIUM_ROOT_DIR}/android/release/lib/arm64-v8a/libpdfium.so")
    elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
        set(PDFIUM_LIBRARY "${PDFIUM_ROOT_DIR}/android/release/lib/armeabi-v7a/libpdfium.so")
    elseif(ANDROID_ABI STREQUAL "x86_64")
        set(PDFIUM_LIBRARY "${PDFIUM_ROOT_DIR}/android/release/lib/x86_64/libpdfium.so")
    elseif(ANDROID_ABI STREQUAL "x86")
        set(PDFIUM_LIBRARY "${PDFIUM_ROOT_DIR}/android/release/lib/x86/libpdfium.so")
    else()
        message(WARNING "Unsupported Android ABI: ${ANDROID_ABI}")
    endif()
    
    set(PDFIUM_INCLUDE_DIR "${PDFIUM_ROOT_DIR}/android/release/include")
    
else()
    message(STATUS "PDFium not configured for this platform")
endif()

# Create PDFium imported target if library exists
if(EXISTS "${PDFIUM_LIBRARY}")
    message(STATUS "✅ Found PDFium library: ${PDFIUM_LIBRARY}")
    message(STATUS "✅ PDFium include directory: ${PDFIUM_INCLUDE_DIR}")
    
    add_library(PDFium::PDFium STATIC IMPORTED)
    set_target_properties(PDFium::PDFium PROPERTIES
        IMPORTED_LOCATION "${PDFIUM_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${PDFIUM_INCLUDE_DIR}"
    )
    
    # For Android shared libraries
    if(ANDROID AND PDFIUM_LIBRARY MATCHES "\\.so$")
        set_property(TARGET PDFium::PDFium PROPERTY TYPE SHARED_LIBRARY)
    endif()
    
    set(PDFIUM_FOUND TRUE)
    message(STATUS "✅ PDFium integration enabled")
else()
    message(WARNING "❌ PDFium library not found at: ${PDFIUM_LIBRARY}")
    set(PDFIUM_FOUND FALSE)
endif()

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