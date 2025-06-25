cmake_minimum_required(VERSION 3.18)

# Add project declaration if building independently
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(LeafraCore VERSION 1.0.0 LANGUAGES CXX C)
    message(STATUS "⚠️  Building LeafraCore independently")
    message(STATUS "⚠️  For full functionality, build from SDK root: cd sdk && cmake -B build -S .")
endif()

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
    src/leafra_unicode.cpp
    src/leafra_unicode_cacher.cpp
    src/leafra_debug.cpp
    src/leafra_filemanager.cpp
)

# Add CoreML source file on Apple platforms
if(APPLE AND TARGET Apple::CoreML)
    list(APPEND LEAFRA_CORE_SOURCES src/leafra_coreml.mm)
endif()

# Add FAISS source file if available
if(TARGET FAISS::FAISS)
    list(APPEND LEAFRA_CORE_SOURCES src/leafra_faiss.cpp)
endif()

# Add LlamaCpp source file if available
if(TARGET LlamaCpp::LlamaCpp)
    list(APPEND LEAFRA_CORE_SOURCES src/leafra_llamacpp.cpp)
endif()

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
    include/leafra/leafra_filemanager.h
    include/leafra/leafra_unicode.h
    )

# Add CoreML header on Apple platforms
if(APPLE AND TARGET Apple::CoreML)
    list(APPEND LEAFRA_CORE_HEADERS include/leafra/leafra_coreml.h)
endif()

# Add FAISS header if available
if(TARGET FAISS::FAISS)
    list(APPEND LEAFRA_CORE_HEADERS include/leafra/leafra_faiss.h)
endif()

# Add LlamaCpp header if available
if(TARGET LlamaCpp::LlamaCpp)
    list(APPEND LEAFRA_CORE_HEADERS include/leafra/leafra_llamacpp.h)
endif()

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
    PUBLIC_HEADER ""
)

# Include directories
target_include_directories(LeafraCore 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Add React Native include paths if building RN bindings (iOS only)
if(LEAFRA_BUILD_RN_BINDINGS AND LEAFRA_PLATFORM_IOS)
    # React Native headers are in the example project's Pods
    set(RN_NODE_MODULES_PATH "${PROJECT_SOURCE_DIR}/../example/Leafra/ios/Pods/Headers/Public")
    
    if(EXISTS "${RN_NODE_MODULES_PATH}")
        target_include_directories(LeafraCore 
            PRIVATE 
                ${PROJECT_SOURCE_DIR}/react-native/ios
                ${RN_NODE_MODULES_PATH}
                ${RN_NODE_MODULES_PATH}/React-Core
        )
        message(STATUS "✅ React Native headers found at: ${RN_NODE_MODULES_PATH}")
        message(STATUS "   React Native bridge integrated into LeafraCore")
    else()
        message(STATUS "⚠️  React Native headers not found at: ${RN_NODE_MODULES_PATH}")
        message(STATUS "   LeafraCore will build without React Native integration")
    endif()
elseif(LEAFRA_BUILD_RN_BINDINGS AND NOT LEAFRA_PLATFORM_IOS)
    message(STATUS "⚠️  React Native bridge skipped - only supported on iOS")
endif()

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

# Link CoreML if available (defined in parent CMakeLists.txt)
if(TARGET Apple::CoreML)
    target_link_libraries(LeafraCore PRIVATE Apple::CoreML)
    target_compile_definitions(LeafraCore PRIVATE LEAFRA_HAS_COREML=1)
    message(STATUS "✅ LeafraCore linked with CoreML")
else()
    message(STATUS "⚠️  Building LeafraCore without CoreML")
endif()

# Link TensorFlow Lite if available (defined in parent CMakeLists.txt)
if(TARGET TensorFlowLite::TensorFlowLite)
    target_link_libraries(LeafraCore PRIVATE TensorFlowLite::TensorFlowLite)
    target_compile_definitions(LeafraCore PRIVATE LEAFRA_HAS_TENSORFLOWLITE=1)
    
    # Link required Apple system frameworks for TensorFlow Lite delegates
    if(APPLE)
        target_link_libraries(LeafraCore PRIVATE 
            "-framework Metal"
            "-framework MetalKit"
            "-framework CoreML"
            "-framework Accelerate"
        )
        message(STATUS "✅ Linked Apple system frameworks for TensorFlow Lite")
    endif()
    
    message(STATUS "✅ LeafraCore linked with TensorFlow Lite")
    message(STATUS "  - Core functionality only (delegates in future update)")
else()
    message(STATUS "⚠️  Building LeafraCore without TensorFlow Lite")
endif()

# Link FAISS if available (defined in parent CMakeLists.txt)
if(TARGET FAISS::FAISS)
    target_link_libraries(LeafraCore PRIVATE FAISS::FAISS)
    target_compile_definitions(LeafraCore PRIVATE LEAFRA_HAS_FAISS=1)
    message(STATUS "✅ LeafraCore linked with FAISS")
else()
    message(STATUS "⚠️  Building LeafraCore without FAISS")
endif()

# Link LlamaCpp if available (defined in parent CMakeLists.txt)
if(TARGET LlamaCpp::LlamaCpp)
    target_link_libraries(LeafraCore PRIVATE LlamaCpp::LlamaCpp)
    target_compile_definitions(LeafraCore PRIVATE LEAFRA_HAS_LLAMACPP=1)
    message(STATUS "✅ LeafraCore linked with LlamaCpp")
else()
    message(STATUS "⚠️  Building LeafraCore without LlamaCpp")
endif()

# Platform-specific libraries for logging
if(APPLE)
    # iOS/macOS - os_log is part of the system, link Foundation framework
    target_link_libraries(LeafraCore PRIVATE "-framework Foundation")
    
    # Configure FileManager to compile as Objective-C++ on Apple platforms
    set_source_files_properties(src/leafra_filemanager.cpp PROPERTIES
        COMPILE_FLAGS "-x objective-c++"
    )
    
    
    # Configure React Native bridge files to compile with ARC if building RN bindings
    if(LEAFRA_BUILD_RN_BINDINGS)
        set_source_files_properties(
            ${PROJECT_SOURCE_DIR}/react-native/ios/LeafraSDKModule.mm
            ${PROJECT_SOURCE_DIR}/react-native/ios/LeafraSDKBridge.mm
            PROPERTIES COMPILE_FLAGS "-fobjc-arc"
        )
        
        # Add weak linking for React Native symbols so framework builds successfully
        # These symbols will be resolved at runtime when loaded into a React Native app
        target_link_libraries(LeafraCore PRIVATE 
            "-Wl,-U,_OBJC_CLASS_$_RCTEventEmitter"
            "-Wl,-U,_OBJC_METACLASS_$_RCTEventEmitter" 
            "-Wl,-U,_RCTRegisterModule"
        )
        message(STATUS "✅ Added weak linking for React Native symbols")
    endif()
    
    # Add weak linking for React Native symbols if building RN bindings
    if(LEAFRA_BUILD_RN_BINDINGS)
        # These symbols will be resolved at runtime when loaded into a React Native app
        target_link_libraries(LeafraCore PRIVATE 
            "-Wl,-U,_OBJC_CLASS_$_RCTEventEmitter"
            "-Wl,-U,_OBJC_METACLASS_$_RCTEventEmitter" 
            "-Wl,-U,_RCTRegisterModule"
        )
        message(STATUS "✅ Added weak linking for React Native symbols")
    endif()
    
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

# For Apple frameworks, manually install headers to Headers/leafra subdirectory
if(APPLE)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/leafra/
        DESTINATION Frameworks/LeafraCore.framework/Headers/leafra
        FILES_MATCHING PATTERN "*.h")
        
    # Copy bundled model files to framework Resources/models directory
    message(STATUS "🔍 Debug: LEAFRA_BUNDLED_MODEL_PATHS = '${LEAFRA_BUNDLED_MODEL_PATHS}'")
    if(DEFINED LEAFRA_BUNDLED_MODEL_PATHS)
        message(STATUS "🔍 Debug: LEAFRA_BUNDLED_MODEL_PATHS is DEFINED")
    else()
        message(STATUS "🔍 Debug: LEAFRA_BUNDLED_MODEL_PATHS is NOT DEFINED")
    endif()
    if(DEFINED LEAFRA_BUNDLED_MODEL_PATHS AND NOT "${LEAFRA_BUNDLED_MODEL_PATHS}" STREQUAL "")
        message(STATUS "🔍 Debug: Entering model bundling section")
        # Convert semicolon-separated list to CMake list
        string(REPLACE ";" "\\;" MODEL_PATHS_LIST "${LEAFRA_BUNDLED_MODEL_PATHS}")
        string(REPLACE "\\;" ";" MODEL_PATHS_LIST "${MODEL_PATHS_LIST}")
        message(STATUS "🔍 Debug: MODEL_PATHS_LIST = ${MODEL_PATHS_LIST}")
        
        set(MODEL_COUNT 0)
        foreach(model_path ${MODEL_PATHS_LIST})
            if(EXISTS "${model_path}")
                get_filename_component(model_filename "${model_path}" NAME)
                if(IS_DIRECTORY "${model_path}")
                    # Handle directories (like .mlmodelc packages)
                    install(DIRECTORY "${model_path}"
                        DESTINATION "Frameworks/LeafraCore.framework/Resources/models"
                        USE_SOURCE_PERMISSIONS)
                    message(STATUS "📦 Bundling model directory: ${model_filename}")
                else()
                    # Handle individual files
                    install(FILES "${model_path}"
                        DESTINATION "Frameworks/LeafraCore.framework/Resources/models"
                        RENAME "${model_filename}")
                    message(STATUS "📦 Bundling model file: ${model_filename}")
                endif()
                math(EXPR MODEL_COUNT "${MODEL_COUNT} + 1")
            else()
                message(WARNING "⚠️  Bundled model file/directory not found: ${model_path}")
            endif()
        endforeach()
        
        message(STATUS "✅ ${MODEL_COUNT} model files will be bundled in framework")
    endif()
endif() 
