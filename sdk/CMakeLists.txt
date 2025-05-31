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

# ==============================================================================
# ICU Integration
# ==============================================================================

set(ICU_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/corecpp/third_party/prebuilt/icu")

# Platform-specific ICU configuration
if(APPLE)
    # iOS/macOS - Use system ICU from platform SDK
    # macOS has libicucore (Apple's bundled ICU) in /usr/lib
    # iOS has ICU in the platform SDK
    
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        # iOS - ICU is available in the iOS SDK
        find_library(ICU_UC_LIBRARY icuuc HINTS ${CMAKE_OSX_SYSROOT}/usr/lib)
        find_library(ICU_I18N_LIBRARY icui18n HINTS ${CMAKE_OSX_SYSROOT}/usr/lib)
        find_library(ICU_DATA_LIBRARY icudata HINTS ${CMAKE_OSX_SYSROOT}/usr/lib)
        find_path(ICU_INCLUDE_DIR unicode/unistr.h HINTS ${CMAKE_OSX_SYSROOT}/usr/include)
        
        if(ICU_UC_LIBRARY AND ICU_I18N_LIBRARY AND ICU_DATA_LIBRARY AND ICU_INCLUDE_DIR)
            message(STATUS "✅ Found iOS ICU libraries:")
            message(STATUS "  - UC: ${ICU_UC_LIBRARY}")
            message(STATUS "  - I18N: ${ICU_I18N_LIBRARY}")
            message(STATUS "  - Data: ${ICU_DATA_LIBRARY}")
            message(STATUS "  - Include: ${ICU_INCLUDE_DIR}")
            
            # Create ICU targets
            add_library(ICU::uc SHARED IMPORTED)
            set_target_properties(ICU::uc PROPERTIES
                IMPORTED_LOCATION "${ICU_UC_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
            )
            
            add_library(ICU::i18n SHARED IMPORTED)
            set_target_properties(ICU::i18n PROPERTIES
                IMPORTED_LOCATION "${ICU_I18N_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
                INTERFACE_LINK_LIBRARIES ICU::uc
            )
            
            add_library(ICU::data SHARED IMPORTED)
            set_target_properties(ICU::data PROPERTIES
                IMPORTED_LOCATION "${ICU_DATA_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
            )
            
            set(ICU_FOUND TRUE)
            set(ICU_TYPE "iOS SDK")
        else()
            message(WARNING "❌ iOS ICU libraries not found")
            set(ICU_FOUND FALSE)
        endif()
    else()
        # macOS - Use libicucore (Apple's bundled ICU)
        find_library(ICU_CORE_LIBRARY icucore HINTS /usr/lib ${CMAKE_OSX_SYSROOT}/usr/lib)
        find_path(ICU_INCLUDE_DIR unicode/ptypes.h HINTS /usr/include ${CMAKE_OSX_SYSROOT}/usr/include)
        
        if(ICU_CORE_LIBRARY AND ICU_INCLUDE_DIR)
            message(STATUS "✅ Found macOS ICU library:")
            message(STATUS "  - ICU Core: ${ICU_CORE_LIBRARY}")
            message(STATUS "  - Include: ${ICU_INCLUDE_DIR}")
            
            # Create a unified ICU target for macOS
            add_library(ICU::core SHARED IMPORTED)
            set_target_properties(ICU::core PROPERTIES
                IMPORTED_LOCATION "${ICU_CORE_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
            )
            
            # Create aliases to match the expected interface
            add_library(ICU::uc ALIAS ICU::core)
            add_library(ICU::i18n ALIAS ICU::core)
            
            set(ICU_FOUND TRUE)
            set(ICU_TYPE "macOS ICU Core")
        else()
            message(WARNING "❌ macOS ICU library (libicucore) not found")
            set(ICU_FOUND FALSE)
        endif()
    endif()
    
    if(ICU_FOUND)
        message(STATUS "✅ ICU integration enabled (${ICU_TYPE})")
    endif()
    
elseif(ANDROID)
    # Android - Use ICU from NDK
    # ICU is available in the Android NDK starting from API level 31
    # For older API levels, we'd need to bundle ICU
    
    if(ANDROID_PLATFORM_LEVEL GREATER_EQUAL 31)
        find_library(ICU_UC_LIBRARY icuuc)
        find_library(ICU_I18N_LIBRARY icui18n)
        find_library(ICU_DATA_LIBRARY icudata)
        find_path(ICU_INCLUDE_DIR unicode/unistr.h)
        
        if(ICU_UC_LIBRARY AND ICU_I18N_LIBRARY AND ICU_DATA_LIBRARY AND ICU_INCLUDE_DIR)
            message(STATUS "✅ Found NDK ICU libraries:")
            message(STATUS "  - UC: ${ICU_UC_LIBRARY}")
            message(STATUS "  - I18N: ${ICU_I18N_LIBRARY}")
            message(STATUS "  - Data: ${ICU_DATA_LIBRARY}")
            message(STATUS "  - Include: ${ICU_INCLUDE_DIR}")
            
            # Create ICU targets
            add_library(ICU::uc SHARED IMPORTED)
            set_target_properties(ICU::uc PROPERTIES
                IMPORTED_LOCATION "${ICU_UC_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
            )
            
            add_library(ICU::i18n SHARED IMPORTED)
            set_target_properties(ICU::i18n PROPERTIES
                IMPORTED_LOCATION "${ICU_I18N_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
                INTERFACE_LINK_LIBRARIES ICU::uc
            )
            
            add_library(ICU::data SHARED IMPORTED)
            set_target_properties(ICU::data PROPERTIES
                IMPORTED_LOCATION "${ICU_DATA_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
            )
            
            set(ICU_FOUND TRUE)
            message(STATUS "✅ ICU integration enabled (NDK libraries)")
        else()
            message(WARNING "❌ NDK ICU libraries not found")
            set(ICU_FOUND FALSE)
        endif()
    else()
        message(WARNING "❌ ICU requires Android API level 31+, current level: ${ANDROID_PLATFORM_LEVEL}")
        message(STATUS "Consider bundling ICU for older Android versions")
        set(ICU_FOUND FALSE)
    endif()
    
elseif(WIN32)
    # Windows - Use bundled libraries
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64-bit
        set(ICU_PLATFORM_DIR "${ICU_ROOT_DIR}/windows/win-x64")
        set(ICU_LIB_DIR "${ICU_PLATFORM_DIR}/lib64")
        set(ICU_BIN_DIR "${ICU_PLATFORM_DIR}/bin64")
        set(ICU_ARCH "x64")
    elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64")
        # ARM64
        set(ICU_PLATFORM_DIR "${ICU_ROOT_DIR}/windows/win-arm64")
        set(ICU_LIB_DIR "${ICU_PLATFORM_DIR}/lib64")
        set(ICU_BIN_DIR "${ICU_PLATFORM_DIR}/bin64")
        set(ICU_ARCH "arm64")
    else()
        # 32-bit
        set(ICU_PLATFORM_DIR "${ICU_ROOT_DIR}/windows/win-x32")
        set(ICU_LIB_DIR "${ICU_PLATFORM_DIR}/lib")
        set(ICU_BIN_DIR "${ICU_PLATFORM_DIR}/bin")
        set(ICU_ARCH "x32")
    endif()
    
    set(ICU_INCLUDE_DIR "${ICU_PLATFORM_DIR}/include")
    
    # ICU library files
    set(ICU_UC_LIBRARY "${ICU_LIB_DIR}/icuuc.lib")
    set(ICU_I18N_LIBRARY "${ICU_LIB_DIR}/icuin.lib")
    set(ICU_DATA_LIBRARY "${ICU_LIB_DIR}/icudt.lib")
    
    # ICU DLL files (use glob pattern to handle version numbers)
    file(GLOB ICU_UC_DLL "${ICU_BIN_DIR}/icuuc*.dll")
    file(GLOB ICU_I18N_DLL "${ICU_BIN_DIR}/icuin*.dll")
    file(GLOB ICU_DATA_DLL "${ICU_BIN_DIR}/icudt*.dll")
    
    if(EXISTS "${ICU_UC_LIBRARY}" AND EXISTS "${ICU_I18N_LIBRARY}" AND EXISTS "${ICU_DATA_LIBRARY}")
        message(STATUS "✅ Found ICU libraries (${ICU_ARCH}):")
        message(STATUS "  - UC: ${ICU_UC_LIBRARY}")
        message(STATUS "  - I18N: ${ICU_I18N_LIBRARY}")
        message(STATUS "  - Data: ${ICU_DATA_LIBRARY}")
        message(STATUS "  - Include: ${ICU_INCLUDE_DIR}")
        
        # Validate DLL files exist
        if(ICU_UC_DLL AND ICU_I18N_DLL AND ICU_DATA_DLL)
            list(GET ICU_UC_DLL 0 ICU_UC_DLL_PATH)
            list(GET ICU_I18N_DLL 0 ICU_I18N_DLL_PATH)
            list(GET ICU_DATA_DLL 0 ICU_DATA_DLL_PATH)
            
            message(STATUS "  - UC DLL: ${ICU_UC_DLL_PATH}")
            message(STATUS "  - I18N DLL: ${ICU_I18N_DLL_PATH}")
            message(STATUS "  - Data DLL: ${ICU_DATA_DLL_PATH}")
            
            # Create ICU targets
            add_library(ICU::uc SHARED IMPORTED)
            set_target_properties(ICU::uc PROPERTIES
                IMPORTED_LOCATION "${ICU_UC_DLL_PATH}"
                IMPORTED_IMPLIB "${ICU_UC_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
            )
            
            add_library(ICU::i18n SHARED IMPORTED)
            set_target_properties(ICU::i18n PROPERTIES
                IMPORTED_LOCATION "${ICU_I18N_DLL_PATH}"
                IMPORTED_IMPLIB "${ICU_I18N_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
                INTERFACE_LINK_LIBRARIES ICU::uc
            )
            
            add_library(ICU::data SHARED IMPORTED)
            set_target_properties(ICU::data PROPERTIES
                IMPORTED_LOCATION "${ICU_DATA_DLL_PATH}"
                IMPORTED_IMPLIB "${ICU_DATA_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
            )
            
            set(ICU_FOUND TRUE)
            message(STATUS "✅ ICU integration enabled (Windows ${ICU_ARCH})")
        else()
            message(WARNING "❌ ICU DLL files not found in: ${ICU_BIN_DIR}")
            set(ICU_FOUND FALSE)
        endif()
    else()
        message(WARNING "❌ ICU libraries not found for Windows ${ICU_ARCH}")
        message(STATUS "Expected libraries in: ${ICU_LIB_DIR}")
        set(ICU_FOUND FALSE)
    endif()
    
else()
    # Linux - Use system libraries
    # Try to find ICU using pkg-config first, then fallback to find_library
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(ICU_UC QUIET icu-uc)
        pkg_check_modules(ICU_I18N QUIET icu-i18n)
    endif()
    
    if(ICU_UC_FOUND AND ICU_I18N_FOUND)
        message(STATUS "✅ Found ICU via pkg-config:")
        message(STATUS "  - UC version: ${ICU_UC_VERSION}")
        message(STATUS "  - I18N version: ${ICU_I18N_VERSION}")
        
        # Create ICU targets
        add_library(ICU::uc SHARED IMPORTED)
        set_target_properties(ICU::uc PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${ICU_UC_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${ICU_UC_LIBRARIES}"
            INTERFACE_COMPILE_OPTIONS "${ICU_UC_CFLAGS_OTHER}"
        )
        
        add_library(ICU::i18n SHARED IMPORTED)
        set_target_properties(ICU::i18n PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${ICU_I18N_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${ICU_I18N_LIBRARIES};ICU::uc"
            INTERFACE_COMPILE_OPTIONS "${ICU_I18N_CFLAGS_OTHER}"
        )
        
        set(ICU_FOUND TRUE)
        message(STATUS "✅ ICU integration enabled (system libraries via pkg-config)")
    else()
        # Fallback to find_library
        find_library(ICU_UC_LIBRARY icuuc)
        find_library(ICU_I18N_LIBRARY icui18n)
        find_library(ICU_DATA_LIBRARY icudata)
        find_path(ICU_INCLUDE_DIR unicode/unistr.h)
        
        if(ICU_UC_LIBRARY AND ICU_I18N_LIBRARY AND ICU_INCLUDE_DIR)
            message(STATUS "✅ Found ICU libraries:")
            message(STATUS "  - UC: ${ICU_UC_LIBRARY}")
            message(STATUS "  - I18N: ${ICU_I18N_LIBRARY}")
            if(ICU_DATA_LIBRARY)
                message(STATUS "  - Data: ${ICU_DATA_LIBRARY}")
            endif()
            message(STATUS "  - Include: ${ICU_INCLUDE_DIR}")
            
            # Create ICU targets
            add_library(ICU::uc SHARED IMPORTED)
            set_target_properties(ICU::uc PROPERTIES
                IMPORTED_LOCATION "${ICU_UC_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
            )
            
            add_library(ICU::i18n SHARED IMPORTED)
            set_target_properties(ICU::i18n PROPERTIES
                IMPORTED_LOCATION "${ICU_I18N_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
                INTERFACE_LINK_LIBRARIES ICU::uc
            )
            
            if(ICU_DATA_LIBRARY)
                add_library(ICU::data SHARED IMPORTED)
                set_target_properties(ICU::data PROPERTIES
                    IMPORTED_LOCATION "${ICU_DATA_LIBRARY}"
                    INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
                )
            endif()
            
            set(ICU_FOUND TRUE)
            message(STATUS "✅ ICU integration enabled (system libraries)")
        else()
            message(WARNING "❌ ICU libraries not found. Please install libicu-dev")
            message(STATUS "Ubuntu/Debian: sudo apt-get install libicu-dev")
            message(STATUS "CentOS/RHEL: sudo yum install libicu-devel")
            message(STATUS "Fedora: sudo dnf install libicu-devel")
            set(ICU_FOUND FALSE)
        endif()
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