cmake_minimum_required(VERSION 3.18)

# Core library sources
set(LEAFRA_CORE_SOURCES
    src/leafra_core.cpp
    src/data_processor.cpp
    src/math_utils.cpp
    src/platform_utils.cpp
    src/logger.cpp
    src/leafra_parsing.cpp
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

# Platform-specific libraries for logging
if(APPLE)
    # iOS/macOS - os_log is part of the system, link Foundation framework
    target_link_libraries(LeafraCore PRIVATE "-framework Foundation")
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