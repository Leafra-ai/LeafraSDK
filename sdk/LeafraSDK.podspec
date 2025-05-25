Pod::Spec.new do |s|
  s.name         = "LeafraSDK"
  s.version      = "1.0.0"
  s.summary      = "Cross-platform C++ SDK for Leafra with React Native bindings"
  s.description  = <<-DESC
                   LeafraSDK is a high-performance cross-platform C++ library that provides
                   data processing, mathematical operations, and utility functions.
                   This pod includes React Native bindings for iOS and macOS.
                   DESC
  s.homepage     = "https://github.com/your-org/LeafraSDK"
  s.license      = { :type => "MIT", :file => "LICENSE" }
  s.author       = { "Your Name" => "your.email@example.com" }
  
  s.source       = { :git => "https://github.com/your-org/LeafraSDK.git", :tag => s.version.to_s }
  
  # Platform support
  s.ios.deployment_target = "15.1"
  s.osx.deployment_target = "12.0"
  
  # Source files
  s.source_files = [
    "corecpp/src/**/*.{cpp,mm}",
    "corecpp/include/**/*.h",
    "ios/**/*.{h,mm}"
  ]
  
  # Public headers
  s.public_header_files = [
    "corecpp/include/**/*.h",
    "ios/**/*.h"
  ]
  
  # Header search paths
  s.header_mappings_dir = "corecpp/include"
  s.preserve_paths = [
    "corecpp/include/**/*.h",
    "ios/**/*.h"
  ]
  
  # Dependencies
  s.dependency "React-Core"
  
  # Compiler settings
  s.pod_target_xcconfig = {
    'CLANG_CXX_LANGUAGE_STANDARD' => 'c++17',
    'CLANG_CXX_LIBRARY' => 'libc++',
    'GCC_PREPROCESSOR_DEFINITIONS' => 'LEAFRA_EXPORTS=1',
    'HEADER_SEARCH_PATHS' => '$(PODS_TARGET_SRCROOT)/corecpp/include',
    'OTHER_CPLUSPLUSFLAGS' => '-fvisibility=hidden -fvisibility-inlines-hidden'
  }
  
  # User target settings
  s.user_target_xcconfig = {
    'HEADER_SEARCH_PATHS' => '$(PODS_TARGET_SRCROOT)/corecpp/include'
  }
  
  # Framework settings
  s.frameworks = ['Foundation', 'CoreFoundation']
  s.libraries = ['c++']
  
  # Module map for proper header exposure
  s.module_map = false
  
  # Prepare command to ensure proper directory structure
  s.prepare_command = <<-CMD
    # Ensure all necessary directories exist
    mkdir -p corecpp/src
    mkdir -p corecpp/include/leafra
    mkdir -p ios
    
    # Create empty implementation files if they don't exist (for initial setup)
    touch corecpp/src/leafra_core.cpp
    touch corecpp/src/math_utils.cpp
    touch corecpp/src/data_processor.cpp
    touch corecpp/src/platform_utils.cpp
  CMD
  
  # Subspecs for modular inclusion
  s.subspec 'Core' do |core|
    core.source_files = [
      "corecpp/src/**/*.cpp",
      "corecpp/include/**/*.h"
    ]
    core.public_header_files = "corecpp/include/**/*.h"
  end
  
  s.subspec 'ReactNative' do |rn|
    rn.dependency 'LeafraSDK/Core'
    rn.dependency 'React-Core'
    
    rn.source_files = "ios/**/*.{h,mm}"
    rn.public_header_files = "ios/**/*.h"
  end
  
  # Default subspecs
  s.default_subspecs = ['Core', 'ReactNative']
  
  # Validation
  s.requires_arc = true
  s.static_framework = true
  
  # Documentation
  s.documentation_url = "https://github.com/your-org/LeafraSDK/blob/main/README.md"
end 