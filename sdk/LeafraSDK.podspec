Pod::Spec.new do |s|
  s.name         = "LeafraSDK"
  s.version      = "1.0.0"
  s.summary      = "Cross-platform C++ SDK for Leafra with React Native bindings"
  s.description  = <<-DESC
                   LeafraSDK is a high-performance cross-platform C++ library that provides
                   data processing, RAG capabilities, vector search, LLM integration, 
                   mathematical operations, and utility functions.
                   This pod includes React Native bindings for iOS and macOS with support for:
                   - PDF processing via PDFium
                   - Text chunking and tokenization
                   - Embedding models (CoreML, TensorFlow Lite)
                   - Vector search with FAISS
                   - LLM inference with llama.cpp
                   - SQLite database operations
                   - Unicode text processing with ICU
                   DESC
  s.homepage     = "https://github.com/your-org/LeafraSDK"
  s.license      = { :type => "MIT", :file => "LICENSE" }
  s.author       = { "Your Name" => "your.email@example.com" }
  
  s.source       = { :git => "https://github.com/your-org/LeafraSDK.git", :tag => s.version.to_s }
  
  # Platform support (matching CMakeLists.txt requirements)
  s.ios.deployment_target = "15.1"
  s.osx.deployment_target = "12.0"
  
  # Source files - Core C++ library
  s.source_files = [
    "corecpp/src/**/*.{cpp,mm}",
    "corecpp/include/**/*.h",
    "react-native/ios/**/*.{h,mm}",
    "react-native/common/src/**/*.{ts,js}"
  ]
  
  # Exclude unit tests and third-party source files (using prebuilt libraries)
  s.exclude_files = [
    "corecpp/src/unit_tests/**/*",
    "corecpp/third_party/*/src/**/*",
    "corecpp/third_party/*/examples/**/*",
    "corecpp/third_party/*/tests/**/*",
    "corecpp/third_party/*/test/**/*"
  ]
  
  # Public headers
  s.public_header_files = [
    "corecpp/include/**/*.h",
    "react-native/ios/**/*.h"
  ]
  
  # Header search paths - extensive paths matching CMakeLists.txt
  s.header_mappings_dir = "corecpp/include"
  s.preserve_paths = [
    "corecpp/include/**/*.h",
    "react-native/ios/**/*.h",
    "corecpp/third_party/prebuilt/**/*"
  ]
  
  # Dependencies
  s.dependency "React-Core"
  
  # Embedding framework configuration (default to CoreML)
  embedding_framework = ENV['LEAFRA_EMBEDDING_FRAMEWORK'] || 'coreml'
  
  # Base preprocessor definitions
  base_definitions = [
    'LEAFRA_EXPORTS=1',
    'LEAFRA_HAS_PDFIUM=1',
    'LEAFRA_HAS_SQLITE=1',
    'LEAFRA_USE_SYSTEM_SQLITE_HEADERS=1',
    'LEAFRA_HAS_SENTENCEPIECE=1',
    'LEAFRA_HAS_ICU=1',
    'LEAFRA_HAS_LLAMACPP=1',
    'LEAFRA_HAS_FAISS=1'
  ]
  
  # Add embedding framework specific flags
  if embedding_framework == 'coreml'
    base_definitions << 'LEAFRA_HAS_COREML=1'
  elsif embedding_framework == 'tflite'
    base_definitions << 'LEAFRA_HAS_TENSORFLOWLITE=1'
  end
  
  # Base header search paths
  base_header_paths = [
    '$(PODS_TARGET_SRCROOT)/corecpp/include',
    '$(PODS_TARGET_SRCROOT)/corecpp/src',
    '$(PODS_TARGET_SRCROOT)/corecpp/third_party/prebuilt/pdfium/paulocoutinhox/ios/release/include'
  ]
  
  # Compiler settings
  s.pod_target_xcconfig = {
    'CLANG_CXX_LANGUAGE_STANDARD' => 'c++17',
    'CLANG_CXX_LIBRARY' => 'libc++',
    'GCC_PREPROCESSOR_DEFINITIONS' => base_definitions.join(' '),
    'HEADER_SEARCH_PATHS' => base_header_paths.join(' '),
    'OTHER_CPLUSPLUSFLAGS' => '-fvisibility=hidden -fvisibility-inlines-hidden',
    'MACOSX_DEPLOYMENT_TARGET' => '12.0',
    'IPHONEOS_DEPLOYMENT_TARGET' => '15.1',
    'ENABLE_BITCODE' => 'NO',
    'SKIP_INSTALL' => 'NO',
    'FRAMEWORK_SEARCH_PATHS' => '$(PODS_TARGET_SRCROOT)/corecpp/third_party/prebuilt/**',
    'HEADER_SEARCH_PATHS[sdk=iphoneos*]' => (base_header_paths + [
      '$(PODS_TARGET_SRCROOT)/corecpp/third_party/prebuilt/faiss-mobile/faiss.xcframework/ios-arm64_arm64e/Headers',
      '$(PODS_TARGET_SRCROOT)/corecpp/third_party/prebuilt/sentencepiece/sentencepiece.xcframework/ios-arm64_arm64e/Headers'
    ]).join(' '),
    'HEADER_SEARCH_PATHS[sdk=iphonesimulator*]' => (base_header_paths + [
      '$(PODS_TARGET_SRCROOT)/corecpp/third_party/prebuilt/faiss-mobile/faiss.xcframework/ios-arm64_x86_64-simulator/Headers',
      '$(PODS_TARGET_SRCROOT)/corecpp/third_party/prebuilt/sentencepiece/sentencepiece.xcframework/ios-arm64_x86_64-simulator/Headers'
    ]).join(' ')
  }
  
  # User target settings
  s.user_target_xcconfig = {
    'HEADER_SEARCH_PATHS' => base_header_paths.join(' '),
    'FRAMEWORK_SEARCH_PATHS' => '$(PODS_TARGET_SRCROOT)/corecpp/third_party/prebuilt/**',
    'HEADER_SEARCH_PATHS[sdk=iphoneos*]' => (base_header_paths + [
      '$(PODS_TARGET_SRCROOT)/corecpp/third_party/prebuilt/faiss-mobile/faiss.xcframework/ios-arm64_arm64e/Headers',
      '$(PODS_TARGET_SRCROOT)/corecpp/third_party/prebuilt/sentencepiece/sentencepiece.xcframework/ios-arm64_arm64e/Headers'
    ]).join(' '),
    'HEADER_SEARCH_PATHS[sdk=iphonesimulator*]' => (base_header_paths + [
      '$(PODS_TARGET_SRCROOT)/corecpp/third_party/prebuilt/faiss-mobile/faiss.xcframework/ios-arm64_x86_64-simulator/Headers',
      '$(PODS_TARGET_SRCROOT)/corecpp/third_party/prebuilt/sentencepiece/sentencepiece.xcframework/ios-arm64_x86_64-simulator/Headers'
    ]).join(' ')
  }
  
  # System frameworks and libraries
  s.frameworks = [
    'Foundation', 
    'CoreFoundation',
    'Accelerate',      # Required by FAISS
    'CoreML'          # Required for CoreML embedding framework
  ]
  
  s.libraries = [
    'c++', 
    'sqlite3',         # SQLite database support
    'icucore'          # ICU Unicode support (Apple's bundled ICU)
  ]
  
  # Platform-specific vendored frameworks and libraries
  s.ios.vendored_frameworks = [
    # PDFium for PDF processing
    'corecpp/third_party/prebuilt/pdfium/paulocoutinhox/ios/release/pdfium.xcframework',
    
    # FAISS for vector search with OpenMP
    'corecpp/third_party/prebuilt/faiss-mobile/faiss.xcframework',
    'corecpp/third_party/prebuilt/faiss-mobile/openmp.xcframework',
    
    # llama.cpp for LLM inference
    'corecpp/third_party/prebuilt/llamacpp/apple/llama.xcframework',
    
    # SentencePiece for tokenization (supports both device and simulator)
    'corecpp/third_party/prebuilt/sentencepiece/sentencepiece.xcframework'
  ]
  
  # Add TensorFlow Lite frameworks if selected
  if embedding_framework == 'tflite'
    s.ios.vendored_frameworks += [
      'corecpp/third_party/prebuilt/tensorflowlite_mybuild/ios_arm64/TensorFlowLiteC.framework',
      'corecpp/third_party/prebuilt/tensorflowlite_mybuild/ios_arm64/TensorFlowLiteCCoreML.framework',
      'corecpp/third_party/prebuilt/tensorflowlite_mybuild/ios_arm64/TensorFlowLiteCMetal.framework',
      'corecpp/third_party/prebuilt/tensorflowlite_mybuild/ios_simulator_arm64/TensorFlowLiteC.framework',
      'corecpp/third_party/prebuilt/tensorflowlite_mybuild/ios_simulator_arm64/TensorFlowLiteCCoreML.framework',
      'corecpp/third_party/prebuilt/tensorflowlite_mybuild/ios_simulator_arm64/TensorFlowLiteCMetal.framework'
    ]
  end
  
  # macOS-specific configurations
  s.osx.vendored_frameworks = [
    # PDFium for PDF processing
    'corecpp/third_party/prebuilt/pdfium/paulocoutinhox/macos/release/pdfium.xcframework',
    
    # FAISS for vector search with OpenMP  
    'corecpp/third_party/prebuilt/faiss-mobile/faiss.xcframework',
    'corecpp/third_party/prebuilt/faiss-mobile/openmp.xcframework',
    
    # llama.cpp for LLM inference
    'corecpp/third_party/prebuilt/llamacpp/apple/llama.xcframework'
  ]
  
  # macOS uses static libraries for SentencePiece (no XCFramework available for macOS)
  s.osx.vendored_libraries = [
    'corecpp/third_party/prebuilt/sentencepiece/macos/lib/libsentencepiece.a',
    'corecpp/third_party/prebuilt/sentencepiece/macos/lib/libsentencepiece_train.a'
  ]
  
  # Module map for proper header exposure
  s.module_map = false
  
  # Prepare command to ensure proper directory structure and dependencies
  s.prepare_command = <<-CMD
    # Ensure all necessary directories exist
    mkdir -p corecpp/src
    mkdir -p corecpp/include/leafra
    mkdir -p react-native/ios
    mkdir -p corecpp/third_party/prebuilt
    
    # Create empty implementation files if they don't exist (for initial setup)
    touch corecpp/src/leafra_core.cpp
    touch corecpp/src/math_utils.cpp
    touch corecpp/src/data_processor.cpp
    touch corecpp/src/platform_utils.cpp
    touch corecpp/src/logger.cpp
    touch corecpp/src/leafra_parsing.cpp
    touch corecpp/src/leafra_parsing_adapter_pdf.cpp
    touch corecpp/src/leafra_parsing_adapter_txt.cpp
    touch corecpp/src/leafra_parsing_adapter_docx.cpp
    touch corecpp/src/leafra_parsing_adapter_excel.cpp
    touch corecpp/src/leafra_sqlite.cpp
    touch corecpp/src/leafra_sentencepiece.cpp
    touch corecpp/src/leafra_chunker.cpp
    touch corecpp/src/leafra_unicode.cpp
    touch corecpp/src/leafra_unicode_cacher.cpp
    touch corecpp/src/leafra_debug.cpp
    touch corecpp/src/leafra_llamacpp.cpp
    touch corecpp/src/leafra_faiss.cpp
    # Print configuration info
    echo "LeafraSDK embedding framework: ${LEAFRA_EMBEDDING_FRAMEWORK:-coreml}"
    echo "Configured for iOS/macOS with comprehensive dependencies"
  CMD
  
  # Subspecs for modular inclusion
  s.subspec 'Core' do |core|
    core.source_files = [
      "corecpp/src/**/*.cpp",
      "corecpp/include/**/*.h"
    ]
    core.exclude_files = "corecpp/src/unit_tests/**/*"
    core.public_header_files = "corecpp/include/**/*.h"
    
    # Core-specific frameworks
    core.frameworks = ['Foundation', 'CoreFoundation', 'Accelerate', 'CoreML']
    core.libraries = ['c++', 'sqlite3', 'icucore']
  end
  
  s.subspec 'ReactNative' do |rn|
    rn.dependency 'LeafraSDK/Core'
    rn.dependency 'React-Core'
    
    rn.source_files = [
      "react-native/ios/**/*.{h,mm}",
      "react-native/common/src/**/*.{ts,js}"
    ]
    rn.public_header_files = "react-native/ios/**/*.h"
  end
  
  s.subspec 'PDFProcessing' do |pdf|
    pdf.dependency 'LeafraSDK/Core'
    pdf.ios.vendored_frameworks = 'corecpp/third_party/prebuilt/pdfium/paulocoutinhox/ios/release/pdfium.xcframework'
    pdf.osx.vendored_frameworks = 'corecpp/third_party/prebuilt/pdfium/paulocoutinhox/macos/release/pdfium.xcframework'
  end
  
  s.subspec 'VectorSearch' do |faiss|
    faiss.dependency 'LeafraSDK/Core'
    faiss.vendored_frameworks = [
      'corecpp/third_party/prebuilt/faiss-mobile/faiss.xcframework',
      'corecpp/third_party/prebuilt/faiss-mobile/openmp.xcframework'
    ]
    faiss.frameworks = ['Accelerate']
  end
  
  s.subspec 'LLMInference' do |llm|
    llm.dependency 'LeafraSDK/Core'
    llm.vendored_frameworks = 'corecpp/third_party/prebuilt/llamacpp/apple/llama.xcframework'
  end
  
  s.subspec 'Tokenization' do |sp|
    sp.dependency 'LeafraSDK/Core'
    sp.ios.vendored_frameworks = 'corecpp/third_party/prebuilt/sentencepiece/sentencepiece.xcframework'
    sp.osx.vendored_libraries = [
      'corecpp/third_party/prebuilt/sentencepiece/macos/lib/libsentencepiece.a',
      'corecpp/third_party/prebuilt/sentencepiece/macos/lib/libsentencepiece_train.a'
    ]
  end
  
  # Default subspecs - include all major components
  s.default_subspecs = ['Core', 'ReactNative', 'PDFProcessing', 'VectorSearch', 'LLMInference', 'Tokenization']
  
  # Validation
  s.requires_arc = true
  s.static_framework = true
  
  # Documentation
  s.documentation_url = "https://github.com/your-org/LeafraSDK/blob/main/README.md"
  
  # Additional validation
  s.swift_version = '5.0'
  
  # Weak framework linking for optional features
  s.weak_frameworks = ['Metal', 'MetalKit']  # For TensorFlow Lite Metal delegate
  
  # Resource bundles for models (if any)
  # s.resource_bundles = {
  #   'LeafraSDKModels' => ['corecpp/third_party/models/**/*']
  # }
end 