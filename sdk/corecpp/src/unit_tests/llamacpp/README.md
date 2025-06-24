# LeafraSDK LlamaCpp Unit Tests

This directory contains comprehensive unit tests for the LeafraSDK LlamaCpp integration, providing complete coverage of the `LlamaCppModel` public API for text generation, chat functionality, and advanced AI features.

## Test Coverage

The test suite provides **100% coverage** of the `LlamaCppModel` public API with **16 comprehensive tests** covering all functionality:

### ✅ **Core Model Management (3 tests)**
- **Model Loading & State** - `load_model()`, `is_loaded()`, `unload()`, basic model info
- **Configuration Access** - `get_config()`, configuration parameter validation
- **Error Handling** - Invalid models, graceful failure, error messages

### ✅ **Text Generation (3 tests)**  
- **Basic Text Generation** - `generate_text()`, synchronous generation with statistics
- **Streaming Text Generation** - `generate_text_stream()`, token callbacks, final notifications
- **Continue Generation** - `continue_generation()`, context continuation, additional prompts

### ✅ **Tokenization & Processing (1 test)**
- **Tokenization & Detokenization** - `tokenize()`, `detokenize()`, `get_token_text()`, round-trip validation

### ✅ **Chat Functionality (3 tests)**
- **Chat Template Functionality** - `generate_chat_response()`, `format_chat_prompt()`, message formatting
- **Chat Streaming** - `generate_chat_response_stream()`, streaming chat with callbacks
- **Chat Template Management** - `set_chat_template()`, `get_chat_template()`, template switching

### ✅ **Advanced Features (3 tests)**
- **System Prompt** - `set_system_prompt()`, system-guided generation
- **Embeddings** - `get_embeddings()`, `supports_embeddings()`, vector representations
- **Perplexity** - `calculate_perplexity()`, text complexity measurement

### ✅ **Context & Configuration (3 tests)**
- **Context Management** - `reset_context()`, `get_context_used()`, context tracking
- **Configuration Updates** - `update_generation_config()`, runtime parameter changes
- **Model Info** - `get_model_info()`, detailed model statistics and capabilities

## Platform Testing Status

### ✅ **TESTED & VERIFIED**
- **macOS (Apple Silicon)** - ✅ All 16 tests passing
  - **Architecture:** arm64 (Apple Silicon M-series)
  - **Framework:** llama.xcframework (prebuilt)
  - **Model:** Llama-3.2-3B-Instruct-Q4_K_M.gguf (3.2B parameters)
  - **Build System:** CMake with prebuilt framework integration

### ✅ **TESTED & VERIFIED**
- **iOS Simulator** - ✅ All 16 tests passing
  - **Architecture:** Universal (arm64 + x86_64)
  - **Framework:** llama.xcframework (properly embedded)
  - **Model:** Llama-3.2-3B-Instruct-Q4_K_M.gguf bundled as app resource
  - **Build System:** CMake with xcframework integration and NSBundle APIs

### ⚠️ **READY BUT UNTESTED**
- **iOS Device** - Build system configured, requires Apple Developer account

### ❌ **NOT YET IMPLEMENTED**
- **Windows** - LlamaCpp implementation exists but no test build system
- **Linux** - LlamaCpp implementation exists but no test build system
- **Android** - LlamaCpp implementation exists but no test build system

> **Note:** The LlamaCpp implementation (`leafra_llamacpp.cpp`) includes cross-platform code, but the unit test build system currently only supports Apple platforms (macOS/iOS).

## Test Model Information

**Model Used:** `sdk/corecpp/third_party/models/llm/unsloth/Llama-3.2-3B-Instruct-Q4_K_M.gguf`
- **Architecture:** Llama 3.2 (3B parameters)
- **Quantization:** Q4_K_M (4-bit quantization, medium quality)
- **Vocabulary:** 128,256 tokens
- **Context Size:** 2,048 tokens
- **Embedding Dimension:** 3,072
- **Chat Template:** Llama 3 format with system/user/assistant roles
- **Features:** Text generation, chat, embeddings, perplexity calculation

## Prerequisites

- **macOS:** Xcode 14+ with Command Line Tools
- **iOS:** iOS 13.0+ (required for `std::filesystem` support)
- **CMake:** 3.14 or later
- **Model File:** Llama-3.2-3B-Instruct-Q4_K_M.gguf must be present

## Building and Running Tests

### macOS Native

#### Build
```bash
# Navigate to the test directory
cd sdk/corecpp/src/unit_tests/llamacpp

# Create build directory
mkdir build_macos && cd build_macos

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build the test suite
make -j$(sysctl -n hw.ncpu)
```

#### Run Tests
```bash
# Run the comprehensive test suite
./test_llamacpp_basic

# Or run with CTest
ctest --verbose
```

**Expected Output:**
```
=== LlamaCpp Model Unit Tests ===
Testing model loading and state...
✅ Model loading and state test passed
Testing tokenization and detokenization...
✅ Tokenization and detokenization test passed
[... 14 more tests ...]

=== Test Results ===
Passed: 16/16
🎉 All tests passed!
```

**Expected iOS Simulator Output:**
```
Testing on iPhone 16 iOS Simulator (iOS 18.3.1)
Bundle path: /Users/.../test_llamacpp_basic.app
Model path: /Users/.../test_llamacpp_basic.app/Llama-3.2-3B-Instruct-Q4_K_M.gguf

=== LlamaCpp Model Unit Tests ===
✅ LlamaCpp backend initialized
✅ Framework loading successful
Testing model loading and state...
✅ Model loading and state test passed
[... all 16 tests ...]

=== Test Results ===
Passed: 16/16
🎉 All tests passed!
```

### iOS Simulator

#### Build
```bash
# Navigate to the test directory
cd sdk/corecpp/src/unit_tests/llamacpp

# Create iOS Simulator build directory
mkdir build_ios_simulator && cd build_ios_simulator

# Configure for iOS Simulator (Universal binary: arm64 + x86_64)
cmake .. \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_SYSROOT=iphonesimulator \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
  -G Xcode

# Build test suite
xcodebuild -project LlamaCppModelTests.xcodeproj \
  -scheme test_llamacpp_basic \
  -configuration Debug \
  -destination 'platform=iOS Simulator,name=iPhone 16,OS=18.3.1' \
  build
```

#### Install & Run Tests
```bash
# Install app on iOS Simulator
xcodebuild -project LlamaCppModelTests.xcodeproj \
  -scheme test_llamacpp_basic \
  -configuration Debug \
  -destination 'platform=iOS Simulator,name=iPhone 16,OS=18.3.1' \
  install

# Run tests on iOS Simulator
xcrun simctl spawn booted /path/to/test_llamacpp_basic.app/test_llamacpp_basic

# Alternative: Launch by bundle identifier (may not show console output)
# xcrun simctl launch --console-pty booted com.leafra.llamacpp.test_llamacpp_basic
```

#### Verify Installation
```bash
# Check if app is installed
xcrun simctl listapps booted | grep llamacpp

# Check app bundle contents
ls -la ~/Library/Developer/CoreSimulator/Devices/[DEVICE_ID]/data/Containers/Bundle/Application/[APP_ID]/test_llamacpp_basic.app/

# Verify framework embedding
ls -la ~/Library/Developer/CoreSimulator/Devices/[DEVICE_ID]/data/Containers/Bundle/Application/[APP_ID]/test_llamacpp_basic.app/Frameworks/

# Verify model bundling (~2GB)
ls -lh ~/Library/Developer/CoreSimulator/Devices/[DEVICE_ID]/data/Containers/Bundle/Application/[APP_ID]/test_llamacpp_basic.app/*.gguf
```

## Test Configuration

### CMake Configuration

#### macOS
- **Deployment Target:** 11.0+ (Apple Silicon support)
- **Architectures:** arm64 (Apple Silicon native)
- **Frameworks:** llama.xcframework (prebuilt)
- **Language:** C++17

#### iOS Simulator
- **Deployment Target:** 13.0+ (required for std::filesystem)
- **Architectures:** Universal (arm64 + x86_64)
- **Frameworks:** Foundation, CoreFoundation, llama.xcframework (embedded)
- **Bundle:** Complete iOS app with embedded framework and model (~2GB)
- **Headers:** Framework headers with `-isystem` for clean compilation
- **Language:** C++17 with Objective-C++ for NSBundle APIs
- **Code Signing:** Automatic framework signing during build

### Model Configuration Used in Tests
```cpp
LlamaCppConfig config;
config.n_ctx = 2048;           // Context size
config.n_predict = 50;         // Default generation length
config.temperature = 0.7f;     // Sampling temperature
config.top_p = 0.9f;          // Nucleus sampling
config.top_k = 40;            // Top-k sampling
config.n_threads = 4;         // Processing threads
config.n_gpu_layers = -1;     // Full GPU offload
```

## Test Features Verified

### ✅ **Text Generation**
- Synchronous and streaming generation
- Context continuation and management
- Token-level control and callbacks
- Generation statistics and performance metrics

### ✅ **Chat Functionality**
- Multi-turn conversations with proper formatting
- System prompts and role-based interactions
- Template auto-detection and management
- Streaming chat responses

### ✅ **Advanced AI Features**
- Text embeddings (3072-dimensional vectors)
- Perplexity calculation for text complexity
- Tokenization with special token handling
- Context state management and reset

### ✅ **Configuration & Management**
- Runtime parameter updates without model reload
- Comprehensive model information retrieval
- Error handling and graceful failure modes
- Resource management and cleanup

## Performance Characteristics

**Typical Test Performance (Apple M1/M2):**
- **Model Loading:** ~400ms (first load, cached thereafter)
- **Text Generation:** ~15-25 tokens/second
- **Embedding Generation:** ~50ms for short text
- **Tokenization:** <1ms for typical sentences
- **Context Operations:** <1ms for reset/info

## Troubleshooting

### Common Issues

#### macOS Issues

**Model Not Found:**
```
Failed to load model: Invalid or missing model file
```
- Ensure `Llama-3.2-3B-Instruct-Q4_K_M.gguf` exists in the models directory
- Check file permissions and path correctness

**Framework Loading Issues:**
```
dyld: Library not loaded: @rpath/llama.framework
```
- Verify code signing of prebuilt framework
- Check framework permissions and quarantine flags

#### iOS Simulator Issues

**Framework Not Embedded:**
```
dyld: Library not loaded: @rpath/llama.framework/llama
```
- The framework must be embedded in the app bundle under `Frameworks/`
- CMake should automatically embed and sign the framework during build
- Check that `llama.framework` exists in the app bundle

**Model Not Found in Bundle:**
```
Model file not found: /Users/.../test_llamacpp_basic.app/model.gguf
```
- Model should be bundled as an app resource (~2GB)
- Use NSBundle APIs to get the correct bundle path on iOS
- Verify model exists in app bundle with correct filename

**App Launch Issues:**
```
Unable to install app: Invalid bundle
```
- Ensure proper code signing of embedded frameworks
- Check that app bundle has valid Info.plist
- Verify bundle identifier matches CMake configuration

**NSBundle Path Issues:**
```
Bundle path detection failed
```
- Ensure Objective-C++ compilation is enabled for iOS
- Check that Foundation framework is properly linked
- Verify `@autoreleasepool` usage for NSBundle APIs

#### General Issues

**Memory Issues:**
```
Failed to allocate memory for model
```
- Ensure sufficient RAM (minimum 4GB recommended for 3B model)
- Check available system memory and close other applications

### Debug Build
```bash
# Build with debug symbols and AddressSanitizer
cmake .. -DCMAKE_BUILD_TYPE=Debug
make

# Run with debugging
lldb ./test_llamacpp_basic
```

## iOS Implementation Details

### Key Features Implemented

#### XCFramework Integration
- **Platform Detection:** Automatic iOS simulator vs device framework selection
- **Universal Binary:** arm64 + x86_64 architecture support for simulators
- **Framework Embedding:** Automatic embedding and code signing during build
- **Imported Targets:** Proper CMake imported framework targets for clean builds

#### Model Resource Bundling
- **App Resource:** 2GB+ model bundled directly in iOS app bundle
- **NSBundle APIs:** Native iOS path resolution using Foundation framework
- **Objective-C++ Bridge:** Minimal ObjC++ only for NSBundle path detection
- **Bundle Path Detection:** Runtime bundle path resolution for model loading

#### Build System Features
- **Cross-Platform:** Single CMakeLists.txt supporting macOS and iOS
- **Code Signing:** Automatic framework signing during post-build
- **RPATH Configuration:** Proper `@executable_path/Frameworks` setup
- **Header Treatment:** System includes (`-isystem`) for clean compilation

### Technical Architecture

#### File Structure
```
build_ios_simulator/
├── LlamaCppModelTests.xcodeproj     # Generated Xcode project
└── Debug-iphonesimulator/
    └── test_llamacpp_basic.app/     # Complete iOS app bundle
        ├── test_llamacpp_basic      # Executable
        ├── Info.plist               # Bundle configuration
        ├── Llama-3.2-3B-Instruct-Q4_K_M.gguf  # Bundled model (~2GB)
        └── Frameworks/
            └── llama.framework/     # Embedded xcframework
                ├── llama            # Framework binary
                ├── Headers/         # Framework headers
                └── Info.plist       # Framework info
```

#### Runtime Model Resolution
```cpp
// iOS-specific model path detection using NSBundle APIs
#if TARGET_OS_IOS
@autoreleasepool {
    NSBundle *bundle = [NSBundle mainBundle];
    NSString *bundlePath = [bundle bundlePath];
    NSString *modelPath = [bundlePath stringByAppendingPathComponent:@"Llama-3.2-3B-Instruct-Q4_K_M.gguf"];
    return std::string([modelPath UTF8String]);
}
#endif
```

## Integration Notes

This test suite validates the complete LlamaCpp integration used by:
- **LeafraCore Framework** - Main SDK integration
- **iOS/macOS Apps** - Native application development with proper bundle handling
- **Server Applications** - Backend AI processing
- **Development Tools** - SDK testing and validation

The tests ensure that all public APIs work correctly across different usage patterns and provide comprehensive validation for production deployment with platform-specific optimizations for iOS resource management. 