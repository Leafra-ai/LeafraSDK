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

# ==============================================================================
# SQLite Integration
# ==============================================================================

set(SQLITE_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/corecpp/third_party/prebuilt/sqlite")

# Platform-specific SQLite configuration
if(APPLE)
    # iOS/macOS - Use system SQLite with system headers
    find_library(SQLITE_LIBRARY sqlite3)
    if(SQLITE_LIBRARY)
        message(STATUS "✅ Found system SQLite library: ${SQLITE_LIBRARY}")
        
        # Use system headers - no custom include directory needed
        add_library(SQLite::SQLite3 SHARED IMPORTED)
        set_target_properties(SQLite::SQLite3 PROPERTIES
            IMPORTED_LOCATION "${SQLITE_LIBRARY}"
        )
        
        set(SQLITE_FOUND TRUE)
        set(SQLITE_USE_SYSTEM_HEADERS TRUE)
        message(STATUS "✅ SQLite integration enabled (system library with system headers)")
    else()
        message(WARNING "❌ System SQLite library not found")
        set(SQLITE_FOUND FALSE)
    endif()
    
elseif(ANDROID)
    # Android - Use bundled AAR with our custom headers
    set(SQLITE_AAR_PATH "${SQLITE_ROOT_DIR}/android/sqlite-android-3490200.aar")
    set(SQLITE_INCLUDE_DIR "${SQLITE_ROOT_DIR}/include")
    
    if(EXISTS "${SQLITE_AAR_PATH}")
        message(STATUS "✅ Found SQLite AAR: ${SQLITE_AAR_PATH}")
        
        # For Android, we'll handle the AAR in the Android-specific build files
        # Here we just mark it as found for the C++ code
        add_library(SQLite::SQLite3 INTERFACE IMPORTED)
        set_target_properties(SQLite::SQLite3 PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${SQLITE_INCLUDE_DIR}"
        )
        
        set(SQLITE_FOUND TRUE)
        set(SQLITE_USE_SYSTEM_HEADERS FALSE)
        message(STATUS "✅ SQLite integration enabled (bundled AAR with custom headers)")
    else()
        message(WARNING "❌ SQLite AAR not found at: ${SQLITE_AAR_PATH}")
        set(SQLITE_FOUND FALSE)
    endif()
    
elseif(WIN32)
    # Windows - Use bundled DLL with our custom headers
    set(SQLITE_INCLUDE_DIR "${SQLITE_ROOT_DIR}/include")
    
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64-bit
        set(SQLITE_DLL_DIR "${SQLITE_ROOT_DIR}/windows/win-x64")
        set(SQLITE_ARCH "x64")
    else()
        # 32-bit
        set(SQLITE_DLL_DIR "${SQLITE_ROOT_DIR}/windows/win-x86")
        set(SQLITE_ARCH "x86")
    endif()
    
    set(SQLITE_DLL_PATH "${SQLITE_DLL_DIR}/sqlite3.dll")
    set(SQLITE_DEF_PATH "${SQLITE_DLL_DIR}/sqlite3.def")
    
    if(EXISTS "${SQLITE_DLL_PATH}" AND EXISTS "${SQLITE_DEF_PATH}")
        message(STATUS "✅ Found SQLite DLL (${SQLITE_ARCH}): ${SQLITE_DLL_PATH}")
        message(STATUS "✅ Found SQLite DEF: ${SQLITE_DEF_PATH}")
        
        # Create import library from DLL and DEF file
        set(SQLITE_LIB_PATH "${CMAKE_CURRENT_BINARY_DIR}/sqlite3.lib")
        
        # Generate import library using lib.exe
        add_custom_command(
            OUTPUT "${SQLITE_LIB_PATH}"
            COMMAND lib /def:"${SQLITE_DEF_PATH}" /out:"${SQLITE_LIB_PATH}" /machine:${SQLITE_ARCH}
            DEPENDS "${SQLITE_DEF_PATH}"
            COMMENT "Generating SQLite import library"
        )
        
        add_custom_target(sqlite_import_lib DEPENDS "${SQLITE_LIB_PATH}")
        
        add_library(SQLite::SQLite3 SHARED IMPORTED)
        set_target_properties(SQLite::SQLite3 PROPERTIES
            IMPORTED_LOCATION "${SQLITE_DLL_PATH}"
            IMPORTED_IMPLIB "${SQLITE_LIB_PATH}"
            INTERFACE_INCLUDE_DIRECTORIES "${SQLITE_INCLUDE_DIR}"
        )
        
        # Ensure import library is built before using SQLite
        add_dependencies(SQLite::SQLite3 sqlite_import_lib)
        
        set(SQLITE_FOUND TRUE)
        set(SQLITE_USE_SYSTEM_HEADERS FALSE)
        message(STATUS "✅ SQLite integration enabled (bundled DLL with custom headers)")
    else()
        message(WARNING "❌ SQLite DLL or DEF not found in: ${SQLITE_DLL_DIR}")
        set(SQLITE_FOUND FALSE)
    endif()
    
else()
    # Linux - Use system dynamic library with system headers
    find_library(SQLITE_LIBRARY sqlite3)
    if(SQLITE_LIBRARY)
        message(STATUS "✅ Found system SQLite library: ${SQLITE_LIBRARY}")
        
        # Use system headers - no custom include directory needed
        add_library(SQLite::SQLite3 SHARED IMPORTED)
        set_target_properties(SQLite::SQLite3 PROPERTIES
            IMPORTED_LOCATION "${SQLITE_LIBRARY}"
        )
        
        set(SQLITE_FOUND TRUE)
        set(SQLITE_USE_SYSTEM_HEADERS TRUE)
        message(STATUS "✅ SQLite integration enabled (system library with system headers)")
    else()
        message(WARNING "❌ System SQLite library not found. Please install libsqlite3-dev")
        set(SQLITE_FOUND FALSE)
    endif()
endif()

# Compiler flags
if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/corecpp/include)

# ==============================================================================
# SentencePiece Integration
# ==============================================================================

set(SENTENCEPIECE_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/corecpp/third_party/prebuilt/sentencepiece")

# Platform-specific SentencePiece configuration
if(APPLE)
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        # iOS - Use XCFramework
        set(SENTENCEPIECE_XCFRAMEWORK_PATH "${SENTENCEPIECE_ROOT_DIR}/sentencepiece.xcframework")
        
        if(EXISTS "${SENTENCEPIECE_XCFRAMEWORK_PATH}")
            message(STATUS "✅ Found SentencePiece XCFramework: ${SENTENCEPIECE_XCFRAMEWORK_PATH}")
            
            # Import the XCFramework
            add_library(SentencePiece::SentencePiece STATIC IMPORTED)
            set_target_properties(SentencePiece::SentencePiece PROPERTIES
                IMPORTED_LOCATION "${SENTENCEPIECE_XCFRAMEWORK_PATH}"
                INTERFACE_INCLUDE_DIRECTORIES "${SENTENCEPIECE_ROOT_DIR}/ios-device/include"
            )
            
            set(SENTENCEPIECE_FOUND TRUE)
            message(STATUS "✅ SentencePiece integration enabled (iOS XCFramework)")
        else()
            message(WARNING "❌ SentencePiece XCFramework not found at: ${SENTENCEPIECE_XCFRAMEWORK_PATH}")
            message(STATUS "Run: cd sdk/corecpp/third_party && make -f Makefile.sentencepiece ios")
            set(SENTENCEPIECE_FOUND FALSE)
        endif()
        
    else()
        # macOS
        set(SENTENCEPIECE_LIBRARY "${SENTENCEPIECE_ROOT_DIR}/macos/lib/libsentencepiece.a")
        set(SENTENCEPIECE_TRAIN_LIBRARY "${SENTENCEPIECE_ROOT_DIR}/macos/lib/libsentencepiece_train.a")
        set(SENTENCEPIECE_INCLUDE_DIR "${SENTENCEPIECE_ROOT_DIR}/macos/include")
        
        if(EXISTS "${SENTENCEPIECE_LIBRARY}" AND EXISTS "${SENTENCEPIECE_TRAIN_LIBRARY}")
            message(STATUS "✅ Found SentencePiece library: ${SENTENCEPIECE_LIBRARY}")
            message(STATUS "✅ Found SentencePiece training library: ${SENTENCEPIECE_TRAIN_LIBRARY}")
            
            # Create a combined target that includes both libraries
            add_library(SentencePiece::SentencePiece STATIC IMPORTED)
            set_target_properties(SentencePiece::SentencePiece PROPERTIES
                IMPORTED_LOCATION "${SENTENCEPIECE_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${SENTENCEPIECE_INCLUDE_DIR}"
                INTERFACE_LINK_LIBRARIES "${SENTENCEPIECE_TRAIN_LIBRARY}"
            )
            
            set(SENTENCEPIECE_FOUND TRUE)
            message(STATUS "✅ SentencePiece integration enabled (macOS with training)")
        else()
            message(WARNING "❌ SentencePiece library not found at: ${SENTENCEPIECE_LIBRARY}")
            message(STATUS "Run: cd sdk/corecpp/third_party && make -f Makefile.sentencepiece macos")
            set(SENTENCEPIECE_FOUND FALSE)
        endif()
    endif()
    
elseif(ANDROID)
    # Android - Select appropriate ABI
    if(ANDROID_ABI STREQUAL "arm64-v8a")
        set(SENTENCEPIECE_LIBRARY "${SENTENCEPIECE_ROOT_DIR}/android-arm64-v8a/lib/libsentencepiece.so")
        set(SENTENCEPIECE_INCLUDE_DIR "${SENTENCEPIECE_ROOT_DIR}/android-arm64-v8a/include")
    elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
        set(SENTENCEPIECE_LIBRARY "${SENTENCEPIECE_ROOT_DIR}/android-armeabi-v7a/lib/libsentencepiece.so")
        set(SENTENCEPIECE_INCLUDE_DIR "${SENTENCEPIECE_ROOT_DIR}/android-armeabi-v7a/include")
    elseif(ANDROID_ABI STREQUAL "x86_64")
        set(SENTENCEPIECE_LIBRARY "${SENTENCEPIECE_ROOT_DIR}/android-x86_64/lib/libsentencepiece.so")
        set(SENTENCEPIECE_INCLUDE_DIR "${SENTENCEPIECE_ROOT_DIR}/android-x86_64/include")
    elseif(ANDROID_ABI STREQUAL "x86")
        set(SENTENCEPIECE_LIBRARY "${SENTENCEPIECE_ROOT_DIR}/android-x86/lib/libsentencepiece.so")
        set(SENTENCEPIECE_INCLUDE_DIR "${SENTENCEPIECE_ROOT_DIR}/android-x86/include")
    else()
        message(WARNING "Unsupported Android ABI for SentencePiece: ${ANDROID_ABI}")
        set(SENTENCEPIECE_FOUND FALSE)
    endif()
    
    if(SENTENCEPIECE_FOUND AND EXISTS "${SENTENCEPIECE_LIBRARY}")
        message(STATUS "✅ Found SentencePiece library (${ANDROID_ABI}): ${SENTENCEPIECE_LIBRARY}")
        
        add_library(SentencePiece::SentencePiece SHARED IMPORTED)
        set_target_properties(SentencePiece::SentencePiece PROPERTIES
            IMPORTED_LOCATION "${SENTENCEPIECE_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SENTENCEPIECE_INCLUDE_DIR}"
        )
        
        set(SENTENCEPIECE_FOUND TRUE)
        message(STATUS "✅ SentencePiece integration enabled (Android ${ANDROID_ABI})")
    else()
        message(WARNING "❌ SentencePiece library not found for Android ${ANDROID_ABI}")
        message(STATUS "Run: cd sdk/corecpp/third_party && make -f Makefile.sentencepiece android")
        set(SENTENCEPIECE_FOUND FALSE)
    endif()
    
elseif(WIN32)
    # Windows - Use DLL
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SENTENCEPIECE_ARCH "x64")
    else()
        set(SENTENCEPIECE_ARCH "x86")
    endif()
    
    set(SENTENCEPIECE_DLL_DIR "${SENTENCEPIECE_ROOT_DIR}/windows-${SENTENCEPIECE_ARCH}")
    set(SENTENCEPIECE_LIBRARY "${SENTENCEPIECE_DLL_DIR}/lib/sentencepiece.lib")
    set(SENTENCEPIECE_DLL "${SENTENCEPIECE_DLL_DIR}/bin/sentencepiece.dll")
    set(SENTENCEPIECE_INCLUDE_DIR "${SENTENCEPIECE_DLL_DIR}/include")
    
    if(EXISTS "${SENTENCEPIECE_LIBRARY}" AND EXISTS "${SENTENCEPIECE_DLL}")
        message(STATUS "✅ Found SentencePiece library (${SENTENCEPIECE_ARCH}): ${SENTENCEPIECE_LIBRARY}")
        message(STATUS "✅ Found SentencePiece DLL: ${SENTENCEPIECE_DLL}")
        
        add_library(SentencePiece::SentencePiece SHARED IMPORTED)
        set_target_properties(SentencePiece::SentencePiece PROPERTIES
            IMPORTED_LOCATION "${SENTENCEPIECE_DLL}"
            IMPORTED_IMPLIB "${SENTENCEPIECE_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SENTENCEPIECE_INCLUDE_DIR}"
        )
        
        set(SENTENCEPIECE_FOUND TRUE)
        message(STATUS "✅ SentencePiece integration enabled (Windows ${SENTENCEPIECE_ARCH})")
    else()
        message(WARNING "❌ SentencePiece library not found for Windows ${SENTENCEPIECE_ARCH}")
        message(STATUS "Run: cd sdk/corecpp/third_party && make -f Makefile.sentencepiece windows")
        set(SENTENCEPIECE_FOUND FALSE)
    endif()
    
else()
    # Linux - Use shared library
    set(SENTENCEPIECE_LIBRARY "${SENTENCEPIECE_ROOT_DIR}/linux/lib/libsentencepiece.so")
    set(SENTENCEPIECE_INCLUDE_DIR "${SENTENCEPIECE_ROOT_DIR}/linux/include")
    
    if(EXISTS "${SENTENCEPIECE_LIBRARY}")
        message(STATUS "✅ Found SentencePiece library: ${SENTENCEPIECE_LIBRARY}")
        
        add_library(SentencePiece::SentencePiece SHARED IMPORTED)
        set_target_properties(SentencePiece::SentencePiece PROPERTIES
            IMPORTED_LOCATION "${SENTENCEPIECE_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SENTENCEPIECE_INCLUDE_DIR}"
        )
        
        set(SENTENCEPIECE_FOUND TRUE)
        message(STATUS "✅ SentencePiece integration enabled (Linux)")
    else()
        message(WARNING "❌ SentencePiece library not found at: ${SENTENCEPIECE_LIBRARY}")
        message(STATUS "Run: cd sdk/corecpp/third_party && make -f Makefile.sentencepiece linux")
        set(SENTENCEPIECE_FOUND FALSE)
    endif()
endif()

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