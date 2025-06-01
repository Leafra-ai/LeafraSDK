cmake_minimum_required(VERSION 3.18)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Core library sources
set(LEAFRA_CORE_SOURCES
    src/leafra_core.cpp
    src/data_processor.cpp
    src/math_utils.cpp
    src/platform_utils.cpp
    src/logger.cpp
    src/leafra_parsing.cpp
    src/leafra_parsing_adapter_pdf.cpp
    src/leafra_parsing_adapter_txt.cpp
    src/leafra_parsing_adapter_docx.cpp
    src/leafra_parsing_adapter_excel.cpp
    src/leafra_sqlite.cpp
    src/leafra_sentencepiece.cpp
    src/leafra_chunker.cpp
    src/leafra_debug.cpp
)

# Core library headers
set(LEAFRA_CORE_HEADERS
    include/leafra/leafra_core.h
    include/leafra/data_processor.h
    include/leafra/math_utils.h
    include/leafra/platform_utils.h
    include/leafra/types.h
    include/leafra/logger.h
    include/leafra/leafra_parsing.h
    include/leafra/leafra_sqlite.h
    include/leafra/leafra_sentencepiece.h
    include/leafra/leafra_chunker.h
    include/leafra/leafra_debug.h
)

# Create the core library
if(LEAFRA_BUILD_SHARED)
    add_library(LeafraCore SHARED ${LEAFRA_CORE_SOURCES} ${LEAFRA_CORE_HEADERS})
else()
    add_library(LeafraCore STATIC ${LEAFRA_CORE_SOURCES} ${LEAFRA_CORE_HEADERS})
endif()

# Set target properties
set_target_properties(LeafraCore PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${LEAFRA_CORE_HEADERS}"
)

# Include directories
target_include_directories(LeafraCore 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link PDFium if available (defined in parent CMakeLists.txt)
if(TARGET PDFium::PDFium)
    target_link_libraries(LeafraCore PRIVATE PDFium::PDFium)
    target_compile_definitions(LeafraCore PRIVATE LEAFRA_HAS_PDFIUM=1)
    message(STATUS "✅ LeafraCore linked with PDFium")
else()
    message(STATUS "⚠️  Building LeafraCore without PDFium")
endif()

# Link SQLite if available (defined in parent CMakeLists.txt)
if(TARGET SQLite::SQLite3)
    target_link_libraries(LeafraCore PRIVATE SQLite::SQLite3)
    target_compile_definitions(LeafraCore PRIVATE LEAFRA_HAS_SQLITE=1)
    
    # Add preprocessor definition for system headers if using system SQLite
    if(SQLITE_USE_SYSTEM_HEADERS)
        target_compile_definitions(LeafraCore PRIVATE LEAFRA_USE_SYSTEM_SQLITE_HEADERS=1)
        message(STATUS "✅ LeafraCore linked with SQLite (using system headers)")
    else()
        message(STATUS "✅ LeafraCore linked with SQLite (using custom headers)")
    endif()
else()
    message(STATUS "⚠️  Building LeafraCore without SQLite")
endif()

# Link SentencePiece if available (defined in parent CMakeLists.txt)
if(TARGET SentencePiece::SentencePiece)
    target_link_libraries(LeafraCore PRIVATE SentencePiece::SentencePiece)
    target_compile_definitions(LeafraCore PRIVATE LEAFRA_HAS_SENTENCEPIECE=1)
    message(STATUS "✅ LeafraCore linked with SentencePiece")
else()
    message(STATUS "⚠️  Building LeafraCore without SentencePiece")
endif()

# Link ICU if available (defined in parent CMakeLists.txt)
if(TARGET ICU::uc AND TARGET ICU::i18n)
    target_link_libraries(LeafraCore PRIVATE ICU::i18n ICU::uc)
    
    # Also link ICU data library if available
    if(TARGET ICU::data)
        target_link_libraries(LeafraCore PRIVATE ICU::data)
    endif()
    
    target_compile_definitions(LeafraCore PRIVATE LEAFRA_HAS_ICU=1)
    message(STATUS "✅ LeafraCore linked with ICU")
else()
    message(STATUS "⚠️  Building LeafraCore without ICU")
endif()

# Platform-specific libraries for logging
if(APPLE)
    # iOS/macOS - os_log is part of the system, link Foundation framework
    target_link_libraries(LeafraCore PRIVATE "-framework Foundation")
    
    # Add Core Graphics and other frameworks required by PDFium
    if(TARGET PDFium::PDFium)
        target_link_libraries(LeafraCore PRIVATE 
            "-framework CoreGraphics"
            "-framework CoreFoundation"
            "-framework CoreText"
        )
        message(STATUS "✅ Linked Apple frameworks for PDFium")
    endif()
elseif(ANDROID)
    # Android - link with log library
    target_link_libraries(LeafraCore PRIVATE log)
elseif(WIN32)
    # Windows - no additional libraries needed for OutputDebugString
endif()

# Compiler definitions
target_compile_definitions(LeafraCore PRIVATE
    LEAFRA_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    LEAFRA_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    LEAFRA_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

if(LEAFRA_BUILD_SHARED)
    target_compile_definitions(LeafraCore 
        PRIVATE LEAFRA_EXPORTS
        PUBLIC LEAFRA_SHARED
    )
endif()

# Platform-specific settings
if(APPLE)
    set_target_properties(LeafraCore PROPERTIES
        FRAMEWORK TRUE
        FRAMEWORK_VERSION A
        MACOSX_FRAMEWORK_IDENTIFIER com.leafra.core
        PUBLIC_HEADER "${LEAFRA_CORE_HEADERS}"
    )
    
    if(LEAFRA_PLATFORM_IOS)
        set_target_properties(LeafraCore PROPERTIES
            XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
            XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ""
        )
    endif()
endif()

# Install targets
install(TARGETS LeafraCore
    EXPORT LeafraSDKTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    FRAMEWORK DESTINATION Frameworks
    PUBLIC_HEADER DESTINATION include/leafra
) 