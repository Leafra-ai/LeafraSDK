require "json"

package = JSON.parse(File.read(File.join(__dir__, "package.json")))
folly_compiler_flags = '-DFOLLY_NO_CONFIG -DFOLLY_MOBILE=1 -DFOLLY_USE_LIBCPP=1'

Pod::Spec.new do |s|
  s.name         = "react-native-leafra-sdk"
  s.version      = package["version"]
  s.summary      = package["description"]
  s.homepage     = package["homepage"]
  s.license      = package["license"]
  s.authors      = package["author"]

  s.platforms    = { :ios => "13.0", :osx => "10.15" }
  s.source       = { :git => "https://github.com/your-org/LeafraSDK.git", :tag => "#{s.version}" }

  # Source files for the React Native bridge
  s.source_files = [
    "ios/**/*.{h,m,mm}",
    "common/src/**/*.{js,ts}"
  ]
  
  # Public header files
  s.public_header_files = [
    "ios/**/*.h"
  ]

  # TypeScript/JavaScript files
  s.preserve_paths = [
    "common/src/**/*",
    "lib/**/*",
    "package.json",
    "tsconfig.json",
    "tsconfig.build.json"
  ]

  # React Native dependency
  s.dependency "React-Core"

  # Framework dependencies - LeafraCore.framework and llama.framework
  s.ios.vendored_frameworks = [
    "ios/LeafraCore.framework",
    "ios/llama.framework"
  ]
  
  s.osx.vendored_frameworks = [
    "macos/LeafraCore.framework", 
    "macos/llama.framework"
  ]



  # System frameworks required by LeafraCore
  s.ios.frameworks = [
    "Foundation",
    "CoreML",
    "CoreGraphics", 
    "CoreFoundation",
    "CoreText",
    "Accelerate",
    "Metal",
    "MetalKit"
  ]
  
  s.osx.frameworks = [
    "Foundation",
    "CoreML", 
    "CoreGraphics",
    "CoreFoundation", 
    "CoreText",
    "Accelerate",
    "Metal",
    "MetalKit"
  ]

  # System libraries
  s.libraries = ["sqlite3", "icucore"]

  # Compiler settings
  s.compiler_flags = folly_compiler_flags + ' -DRCT_NEW_ARCH_ENABLED=1'
  s.pod_target_xcconfig = {
    "HEADER_SEARCH_PATHS" => [
      "\"$(PODS_ROOT)/boost\"",
      "\"$(PODS_ROOT)/boost-for-react-native\"",
      "\"$(PODS_ROOT)/DoubleConversion\"",
      "\"$(PODS_ROOT)/RCT-Folly\"",
      "\"$(PODS_ROOT)/Headers/Public/React-Core\"",
      "\"$(PODS_ROOT)/Headers/Public/React-RCTFabric\"",
      "\"$(PODS_ROOT)/Headers/Public/ReactCommon\"",
      "\"$(PODS_TARGET_SRCROOT)/ios/LeafraCore.framework/Headers\"",
      "\"$(PODS_TARGET_SRCROOT)/macos/LeafraCore.framework/Headers\""
    ].join(" "),
    "FRAMEWORK_SEARCH_PATHS" => [
      "\"$(PODS_TARGET_SRCROOT)/ios\"",
      "\"$(PODS_TARGET_SRCROOT)/macos\""
    ].join(" "),
    "CLANG_CXX_LANGUAGE_STANDARD" => "c++17",
    "CLANG_CXX_LIBRARY" => "libc++",
    "OTHER_CPLUSPLUSFLAGS" => "-DFOLLY_NO_CONFIG -DFOLLY_MOBILE=1 -DFOLLY_USE_LIBCPP=1",
    "OTHER_CFLAGS" => "$(inherited)"
  }

  # Install React Native dependency based on React Native version
  if respond_to?(:install_modules_dependencies, true)
    install_modules_dependencies(s)
  else
    s.dependency "React-Core"

    # Don't install the dependencies when we run `pod install` in the old architecture.
    if ENV['RCT_NEW_ARCH_ENABLED'] == '1' then
      s.compiler_flags = folly_compiler_flags + " -DRCT_NEW_ARCH_ENABLED=1"
      s.pod_target_xcconfig    = {
        "HEADER_SEARCH_PATHS" => "\"$(PODS_ROOT)/boost\"",
        "OTHER_CPLUSPLUSFLAGS" => "-DFOLLY_NO_CONFIG -DFOLLY_MOBILE=1 -DFOLLY_USE_LIBCPP=1",
        "CLANG_CXX_LANGUAGE_STANDARD" => "c++17"
      }
      s.dependency "React-Codegen"
      s.dependency "RCT-Folly"
      s.dependency "RCTRequired"
      s.dependency "RCTTypeSafety"
      s.dependency "ReactCommon/turbomodule/core"
    end
  end

  # Exclude files from build
  s.exclude_files = [
    "android/**/*",
    "windows/**/*",
    "**/__tests__/**/*",
    "**/__fixtures__/**/*", 
    "**/__mocks__/**/*",
    "**/.*"
  ]

  # Resource bundles - include the LeafraResources.bundle
  s.ios.resource_bundles = {
    'LeafraSDKResources' => ['ios/LeafraCore.framework/LeafraResources.bundle/**/*']
  }
  
  s.osx.resource_bundles = {
    'LeafraSDKResources' => ['macos/LeafraCore.framework/LeafraResources.bundle/**/*']
  }

  # Minimum deployment targets
  s.ios.deployment_target = "13.0"
  s.osx.deployment_target = "10.15"

  # Module map for proper Swift/ObjC interop
  s.module_map = "ios/LeafraSDK.modulemap"

  # Description and metadata
  s.description = <<-DESC
    React Native bindings for LeafraSDK - A high-performance cross-platform C++ library
    for data processing, mathematical operations, document parsing, semantic search,
    and AI/ML inference capabilities.
    
    Features:
    - Document parsing (PDF, DOCX, TXT, Excel)
    - Mathematical operations and utilities
    - Semantic search with vector databases
    - AI/ML inference (CoreML, LlamaCpp)
    - Cross-platform C++ core with React Native bindings
    
    This package provides the iOS and macOS React Native bridge to the LeafraSDK.
  DESC

  s.requires_arc = true
end 