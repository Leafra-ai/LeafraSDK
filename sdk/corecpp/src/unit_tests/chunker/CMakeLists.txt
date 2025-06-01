cmake_minimum_required(VERSION 3.14)

# Unit tests for LeafraChunker
project(LeafraChunkerTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(../../../include)

# ==============================================================================
# Configure Dependencies (PDFium, SQLite, SentencePiece)
# ==============================================================================

# PDFium Integration
set(PDFIUM_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/prebuilt/pdfium/paulocoutinhox")

if(APPLE)
    # macOS
    set(PDFIUM_LIBRARY "${PDFIUM_ROOT_DIR}/macos/release/lib/libpdfium.a")
    set(PDFIUM_INCLUDE_DIR "${PDFIUM_ROOT_DIR}/macos/release/include")
    
    if(EXISTS "${PDFIUM_LIBRARY}")
        message(STATUS "✅ Found PDFium library: ${PDFIUM_LIBRARY}")
        add_library(PDFium::PDFium STATIC IMPORTED)
        set_target_properties(PDFium::PDFium PROPERTIES
            IMPORTED_LOCATION "${PDFIUM_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${PDFIUM_INCLUDE_DIR}"
        )
        set(PDFIUM_FOUND TRUE)
    else()
        message(WARNING "❌ PDFium library not found")
        set(PDFIUM_FOUND FALSE)
    endif()
endif()

# SQLite Integration
if(APPLE)
    find_library(SQLITE_LIBRARY sqlite3)
    if(SQLITE_LIBRARY)
        add_library(SQLite::SQLite3 SHARED IMPORTED)
        set_target_properties(SQLite::SQLite3 PROPERTIES
            IMPORTED_LOCATION "${SQLITE_LIBRARY}"
        )
        set(SQLITE_FOUND TRUE)
    endif()
endif()

# SentencePiece Integration
set(SENTENCEPIECE_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/prebuilt/sentencepiece")

if(APPLE)
    set(SENTENCEPIECE_LIBRARY "${SENTENCEPIECE_ROOT_DIR}/macos/lib/libsentencepiece.a")
    set(SENTENCEPIECE_TRAIN_LIBRARY "${SENTENCEPIECE_ROOT_DIR}/macos/lib/libsentencepiece_train.a")
    set(SENTENCEPIECE_INCLUDE_DIR "${SENTENCEPIECE_ROOT_DIR}/macos/include")
    
    if(EXISTS "${SENTENCEPIECE_LIBRARY}" AND EXISTS "${SENTENCEPIECE_TRAIN_LIBRARY}")
        message(STATUS "✅ Found SentencePiece library: ${SENTENCEPIECE_LIBRARY}")
        add_library(SentencePiece::SentencePiece STATIC IMPORTED)
        set_target_properties(SentencePiece::SentencePiece PROPERTIES
            IMPORTED_LOCATION "${SENTENCEPIECE_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SENTENCEPIECE_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "${SENTENCEPIECE_TRAIN_LIBRARY}"
        )
        set(SENTENCEPIECE_FOUND TRUE)
    else()
        message(WARNING "❌ SentencePiece library not found")
        set(SENTENCEPIECE_FOUND FALSE)
    endif()
endif()

# ICU Integration
if(APPLE)
    # Use macOS ICU Core
    find_library(ICU_CORE_LIBRARY icucore)
    find_path(ICU_INCLUDE_DIR unicode/ptypes.h)
    
    if(ICU_CORE_LIBRARY AND ICU_INCLUDE_DIR)
        message(STATUS "✅ Found macOS ICU library: ${ICU_CORE_LIBRARY}")
        add_library(ICU::core SHARED IMPORTED)
        set_target_properties(ICU::core PROPERTIES
            IMPORTED_LOCATION "${ICU_CORE_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIR}"
        )
        # Create aliases for compatibility
        add_library(ICU::uc ALIAS ICU::core)
        add_library(ICU::i18n ALIAS ICU::core)
        set(ICU_FOUND TRUE)
    else()
        message(WARNING "❌ ICU library not found")
        set(ICU_FOUND FALSE)
    endif()
endif()

# Common source files for tests
set(COMMON_SOURCES
    ../../../src/leafra_chunker.cpp
    ../../../src/leafra_unicode.cpp
    ../../../src/leafra_parsing.cpp
    ../../../src/leafra_parsing_adapter_pdf.cpp
    ../../../src/leafra_parsing_adapter_txt.cpp
    ../../../src/leafra_parsing_adapter_docx.cpp
    ../../../src/leafra_parsing_adapter_excel.cpp
    ../../../src/logger.cpp
    ../../../src/leafra_debug.cpp
)

# Add individual test executables
add_executable(test_leafra_chunker 
    test_leafra_chunker.cpp
    ${COMMON_SOURCES}
)

add_executable(test_token_chunking 
    test_token_chunking.cpp
    ${COMMON_SOURCES}
)

add_executable(test_unified_api 
    test_unified_api.cpp
    ${COMMON_SOURCES}
)

add_executable(test_advanced_chunking 
    test_advanced_chunking.cpp
    ${COMMON_SOURCES}
)

add_executable(test_utf8_chunking 
    test_utf8_chunking.cpp
    ${COMMON_SOURCES}
)

add_executable(test_debug_functionality 
    test_debug_functionality.cpp
    ${COMMON_SOURCES}
)

# Platform-specific linking
if(APPLE)
    # Link all targets with the required libraries
    set(APPLE_LINK_LIBRARIES
        "-framework Foundation"
        "-framework CoreFoundation"
    )
    
    # Add PDFium if available
    if(PDFIUM_FOUND)
        list(APPEND APPLE_LINK_LIBRARIES 
            PDFium::PDFium
            "-framework CoreGraphics"
            "-framework CoreText"
        )
    endif()
    
    # Add SQLite if available
    if(SQLITE_FOUND)
        list(APPEND APPLE_LINK_LIBRARIES SQLite::SQLite3)
    endif()
    
    # Add SentencePiece if available
    if(SENTENCEPIECE_FOUND)
        list(APPEND APPLE_LINK_LIBRARIES SentencePiece::SentencePiece)
    endif()
    
    # Add ICU if available
    if(ICU_FOUND)
        list(APPEND APPLE_LINK_LIBRARIES ICU::core)
    endif()
    
    # Apply to all test targets
    target_link_libraries(test_leafra_chunker ${APPLE_LINK_LIBRARIES})
    target_link_libraries(test_token_chunking ${APPLE_LINK_LIBRARIES})
    target_link_libraries(test_unified_api ${APPLE_LINK_LIBRARIES})
    target_link_libraries(test_advanced_chunking ${APPLE_LINK_LIBRARIES})
    target_link_libraries(test_utf8_chunking ${APPLE_LINK_LIBRARIES})
    target_link_libraries(test_debug_functionality ${APPLE_LINK_LIBRARIES})
    
    # Add compile definitions for feature flags
    if(PDFIUM_FOUND)
        target_compile_definitions(test_leafra_chunker PRIVATE LEAFRA_HAS_PDFIUM=1)
        target_compile_definitions(test_token_chunking PRIVATE LEAFRA_HAS_PDFIUM=1)
        target_compile_definitions(test_unified_api PRIVATE LEAFRA_HAS_PDFIUM=1)
        target_compile_definitions(test_advanced_chunking PRIVATE LEAFRA_HAS_PDFIUM=1)
        target_compile_definitions(test_utf8_chunking PRIVATE LEAFRA_HAS_PDFIUM=1)
        target_compile_definitions(test_debug_functionality PRIVATE LEAFRA_HAS_PDFIUM=1)
    endif()
    
    if(SQLITE_FOUND)
        target_compile_definitions(test_leafra_chunker PRIVATE LEAFRA_HAS_SQLITE=1 LEAFRA_USE_SYSTEM_SQLITE_HEADERS=1)
        target_compile_definitions(test_token_chunking PRIVATE LEAFRA_HAS_SQLITE=1 LEAFRA_USE_SYSTEM_SQLITE_HEADERS=1)
        target_compile_definitions(test_unified_api PRIVATE LEAFRA_HAS_SQLITE=1 LEAFRA_USE_SYSTEM_SQLITE_HEADERS=1)
        target_compile_definitions(test_advanced_chunking PRIVATE LEAFRA_HAS_SQLITE=1 LEAFRA_USE_SYSTEM_SQLITE_HEADERS=1)
        target_compile_definitions(test_utf8_chunking PRIVATE LEAFRA_HAS_SQLITE=1 LEAFRA_USE_SYSTEM_SQLITE_HEADERS=1)
        target_compile_definitions(test_debug_functionality PRIVATE LEAFRA_HAS_SQLITE=1 LEAFRA_USE_SYSTEM_SQLITE_HEADERS=1)
    endif()
    
    if(SENTENCEPIECE_FOUND)
        target_compile_definitions(test_leafra_chunker PRIVATE LEAFRA_HAS_SENTENCEPIECE=1)
        target_compile_definitions(test_token_chunking PRIVATE LEAFRA_HAS_SENTENCEPIECE=1)
        target_compile_definitions(test_unified_api PRIVATE LEAFRA_HAS_SENTENCEPIECE=1)
        target_compile_definitions(test_advanced_chunking PRIVATE LEAFRA_HAS_SENTENCEPIECE=1)
        target_compile_definitions(test_utf8_chunking PRIVATE LEAFRA_HAS_SENTENCEPIECE=1)
        target_compile_definitions(test_debug_functionality PRIVATE LEAFRA_HAS_SENTENCEPIECE=1)
    endif()
    
    if(ICU_FOUND)
        target_compile_definitions(test_leafra_chunker PRIVATE LEAFRA_HAS_ICU=1)
        target_compile_definitions(test_token_chunking PRIVATE LEAFRA_HAS_ICU=1)
        target_compile_definitions(test_unified_api PRIVATE LEAFRA_HAS_ICU=1)
        target_compile_definitions(test_advanced_chunking PRIVATE LEAFRA_HAS_ICU=1)
        target_compile_definitions(test_utf8_chunking PRIVATE LEAFRA_HAS_ICU=1)
        target_compile_definitions(test_debug_functionality PRIVATE LEAFRA_HAS_ICU=1)
    endif()
endif()

# Optional: Add a target to run all tests
add_custom_target(run_chunker_tests
    COMMAND test_leafra_chunker
    COMMAND test_token_chunking
    COMMAND test_unified_api
    COMMAND test_advanced_chunking
    COMMAND test_utf8_chunking
    COMMAND test_debug_functionality
    DEPENDS test_leafra_chunker test_token_chunking test_unified_api test_advanced_chunking test_utf8_chunking test_debug_functionality
    COMMENT "Running all chunker tests"
)

# Enable testing
enable_testing()
add_test(NAME BasicChunkerTest COMMAND test_leafra_chunker)
add_test(NAME TokenChunkingTest COMMAND test_token_chunking)
add_test(NAME UnifiedAPITest COMMAND test_unified_api)
add_test(NAME AdvancedChunkingTest COMMAND test_advanced_chunking)
add_test(NAME UTF8ChunkingTest COMMAND test_utf8_chunking)

# Platform-specific settings
if(APPLE)
    # macOS/iOS specific settings
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        set_target_properties(test_leafra_chunker PROPERTIES
            XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
            XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ""
        )
    endif()
elseif(WIN32)
    # Windows specific settings
    if(MSVC)
        target_compile_options(test_leafra_chunker PRIVATE /W4)
    endif()
elseif(ANDROID)
    # Android specific settings
    target_link_libraries(test_leafra_chunker log)
elseif(UNIX)
    # Linux specific settings
    target_compile_options(test_leafra_chunker PRIVATE -Wall -Wextra)
endif()

# Compiler definitions for cross-platform compatibility
target_compile_definitions(test_leafra_chunker PRIVATE
    $<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS>
    $<$<PLATFORM_ID:Android>:__ANDROID__>
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
install(TARGETS test_leafra_chunker test_token_chunking test_unified_api test_advanced_chunking test_utf8_chunking test_debug_functionality
    RUNTIME DESTINATION bin/tests
) 