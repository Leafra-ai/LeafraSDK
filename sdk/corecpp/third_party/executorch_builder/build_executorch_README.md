# Executorch Prebuilt Libraries Build Guide

This guide explains how to build Executorch prebuilt libraries from source with the correct backend support for each platform.

**Location**: This build system is located in `sdk/corecpp/third_party/executorch_builder/`  
**Source**: Executorch source code is at `sdk/corecpp/third_party/executorch/`

## Prerequisites

### Required Tools
- **CMake 3.24+**
- **Python 3.8+** with pip
- **Git** with submodules support
- **Platform-specific toolchains** (see per-platform sections)

### Required Python Packages
```bash
pip3 install zstd
```

### Initial Setup
```bash
# Navigate to executorch source directory
cd sdk/corecpp/third_party/executorch

# Initialize submodules (this takes a few minutes)
git submodule sync
git submodule update --init

# Clean any previous builds
./install_executorch.sh --clean
```

## Quick Start (Using Build System)

Navigate to the build system directory:
```bash
cd sdk/corecpp/third_party/executorch_builder
```

### Build All Apple Platforms (Recommended)
```bash
# Build iOS, iOS Simulator, and macOS with all backends and optimized kernels
make -f Makefile.platforms apple
```

### Other Platforms
```bash
# Configure Android build environment
make -f Makefile.platforms android-config

# Build for Linux
make -f Makefile.platforms linux

# Build for Windows  
make -f Makefile.platforms windows

# Clean all builds
make -f Makefile.platforms clean

# Show help
make -f Makefile.platforms help
```

## iOS and macOS Builds (XCFrameworks)

### Supported Backends
- ✅ **XNNPACK** - High-performance neural network operators
- ✅ **CoreML** - Apple's machine learning framework
- ✅ **MPS** - Metal Performance Shaders for GPU acceleration
- ✅ **Portable Kernels** - Fallback CPU implementations
- ✅ **Optimized Kernels** - Platform-optimized CPU kernels

### Prerequisites
- **Xcode 14+** with command line tools
- **macOS 12+**
- **iOS 15+ / macOS 10.15+ deployment targets**

### Build Command (Manual)
```bash
# Navigate to executorch source
cd sdk/corecpp/third_party/executorch

# Build XCFrameworks for iOS (device + simulator) and macOS
./scripts/build_apple_frameworks.sh \
    --output=apple-frameworks \
    --Release \
    --coreml \
    --mps \
    --xnnpack \
    --portable \
    --optimized

# Output location: apple-frameworks/
# - executorch.xcframework
# - backend_coreml.xcframework  
# - backend_mps.xcframework
# - backend_xnnpack.xcframework
# - kernels_portable.xcframework
# - kernels_optimized.xcframework
```

### Integration
Add the XCFrameworks to your Xcode project:
1. Drag and drop the `.xcframework` files into your project
2. Ensure they're linked in "Frameworks, Libraries, and Embedded Content"
3. Set "Embed & Sign" for each framework

## C++ Bridge for iOS/macOS XCFrameworks

The build system now generates C++ bridge files that allow you to use ExecutTorch from C++ code while leveraging the prebuilt iOS/macOS XCFrameworks.

### Bridge Location
- **Bridge Files**: `sdk/corecpp/third_party/executorch_c++_bridge/`
- **XCFrameworks**: `sdk/corecpp/third_party/prebuilt/executorch/apple/`

### Building with C++ Bridge
```bash
# Build XCFrameworks and generate C++ bridge
cd sdk/corecpp/third_party/executorch_builder
make -f Makefile.platforms apple
```

### Using ExecutTorch from C++

#### 1. Include Headers
```cpp
#include "ExecutTorch_Bridge.h"  // Main C bridge API (includes all components)
```

#### 2. Basic C++ Usage Example
```cpp
#include "ExecutTorch_Bridge.h"
#include <iostream>

int main() {
    // Create module
    ExecuTorchModule_Handle module = ExecuTorchModule_createWithDefaultMode("path/to/model.pte");
    if (!module) {
        std::cerr << "Failed to create module" << std::endl;
        return 1;
    }
    
    // Load module
    char* error = nullptr;
    if (!ExecuTorchModule_load(module, &error)) {
        std::cerr << "Failed to load module: " << error << std::endl;
        free(error);
        ExecuTorchModule_destroy(module);
        return 1;
    }
    
    // Create input tensor
    float inputData[] = {1.0, 2.0, 3.0, 4.0};
    int64_t shape[] = {2, 2};
    ExecuTorchTensor_Handle tensor = ExecuTorchTensor_createSimple(
        inputData, shape, 2, ET_DATA_TYPE_FLOAT);
    
    // Create value from tensor
    ExecuTorchValue_Handle value = ExecuTorchValue_createWithTensor(tensor);
    
    // Execute method
    size_t outputCount;
    ExecuTorchValue_Handle* outputs = ExecuTorchModule_executeMethod(
        module, "forward", &value, 1, &outputCount, &error);
    
    if (outputs) {
        std::cout << "Got " << outputCount << " outputs" << std::endl;
        
        // Process outputs
        for (size_t i = 0; i < outputCount; i++) {
            if (ExecuTorchValue_isTensor(outputs[i])) {
                ExecuTorchTensor_Handle outTensor = ExecuTorchValue_getTensor(outputs[i]);
                size_t dims;
                const int64_t* outShape = ExecuTorchTensor_getShape(outTensor, &dims);
                std::cout << "Output " << i << " shape: [";
                for (size_t j = 0; j < dims; j++) {
                    std::cout << outShape[j];
                    if (j < dims - 1) std::cout << ", ";
                }
                std::cout << "]" << std::endl;
            }
        }
        
        ExecuTorchModule_freeOutputs(outputs, outputCount);
    } else {
        std::cerr << "Execution failed: " << error << std::endl;
        free(error);
    }
    
    // Cleanup
    ExecuTorchValue_destroy(value);
    ExecuTorchTensor_destroy(tensor);
    ExecuTorchModule_destroy(module);
    
    return 0;
}
```

#### 3. Compilation Instructions

To compile your C++ code with the ExecutTorch bridge:

```bash
# Basic compilation
clang++ -std=c++14 \
    -I/path/to/executorch_c++_bridge \
    -I../prebuilt/executorch/apple/executorch.xcframework/macos-arm64/Headers \
    -F../prebuilt/executorch/apple \
    -framework Foundation \
    -framework executorch \
    your_app.cpp \
    /path/to/executorch_c++_bridge/*.mm \
    -o your_app

# For iOS builds, use iOS headers:
clang++ -std=c++14 \
    -I/path/to/executorch_c++_bridge \
    -I../prebuilt/executorch/apple/executorch.xcframework/ios-arm64/Headers \
    -F../prebuilt/executorch/apple \
    -framework Foundation \
    -framework executorch \
    your_app.cpp \
    /path/to/executorch_c++_bridge/*.mm \
    -o your_app
```

#### 4. Xcode Integration

When integrating with Xcode projects:

1. **Add XCFrameworks**: Drag the `.xcframework` files to your Xcode project:
   - `executorch.xcframework`
   - `backend_coreml.xcframework`
   - `backend_mps.xcframework`  
   - `backend_xnnpack.xcframework`
   - `kernels_portable.xcframework`
   - `kernels_optimized.xcframework`

2. **Add Bridge Files**: Include the C++ bridge files in your project:
   - All `.h` and `.mm` files from `executorch_c++_bridge/`

3. **Compiler Settings**:
   - Set file types: `.mm` files should be compiled as Objective-C++
   - Enable C++ exceptions: `-fexceptions`
   - C++ standard: C++14 or later

4. **Build Settings**:
   ```
   OTHER_CPLUSPLUSFLAGS = -std=c++14 -fexceptions
   CLANG_ENABLE_OBJC_ARC = YES
   ```

#### 5. CMake Integration

For CMake projects:

```cmake
# Find the bridge files
set(EXECUTORCH_BRIDGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdk/corecpp/third_party/executorch_c++_bridge")
set(EXECUTORCH_XCFRAMEWORK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdk/corecpp/third_party/prebuilt/executorch/apple")

# Add bridge sources
file(GLOB EXECUTORCH_BRIDGE_SOURCES 
    "${EXECUTORCH_BRIDGE_DIR}/*.mm"
    "${EXECUTORCH_BRIDGE_DIR}/*.h"
)

# Create library
add_library(executorch_bridge ${EXECUTORCH_BRIDGE_SOURCES})

# Link XCFrameworks
target_link_libraries(executorch_bridge
    "${EXECUTORCH_XCFRAMEWORK_DIR}/executorch.xcframework"
    "${EXECUTORCH_XCFRAMEWORK_DIR}/backend_coreml.xcframework"
    "${EXECUTORCH_XCFRAMEWORK_DIR}/backend_mps.xcframework"
    "${EXECUTORCH_XCFRAMEWORK_DIR}/backend_xnnpack.xcframework"
    "${EXECUTORCH_XCFRAMEWORK_DIR}/kernels_portable.xcframework"
    "${EXECUTORCH_XCFRAMEWORK_DIR}/kernels_optimized.xcframework"
    "-framework Foundation"
)

# Set properties
set_target_properties(executorch_bridge PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

# Your app
add_executable(my_app main.cpp)
target_link_libraries(my_app executorch_bridge)
target_include_directories(my_app PRIVATE ${EXECUTORCH_BRIDGE_DIR})
```

#### 4. Available Bridge Components

- **ExecuTorch_Bridge.h**: Main C bridge header (includes all components)
- **ExecuTorchModule_Bridge.h/.mm**: Module loading and execution
- **ExecuTorchTensor_Bridge.h/.mm**: Tensor creation and manipulation  
- **ExecuTorchValue_Bridge.h/.mm**: Value types (tensors, scalars, strings)
- **ExecuTorchLog_Bridge.h/.mm**: Logging functionality
- **ExecuTorchError_Bridge.h/.mm**: Error handling
- **example_usage.cpp**: Complete working example

## Android Builds

### Supported Backends
- ✅ **XNNPACK** - Cross-platform neural network operators
- ✅ **Vulkan** - Cross-platform GPU API
- ✅ **QNN** - Qualcomm Neural Processing SDK (NPU)
- ✅ **Neuron** - MediaTek NeuroPilot (NPU)

### Prerequisites
- **Android NDK r25+**
- **Android SDK API 26+**
- **CMake 3.24+**

### Environment Setup
```bash
# From executorch_builder directory
cd sdk/corecpp/third_party/executorch_builder

# Set Android environment variables
export ANDROID_HOME=/path/to/android/sdk
export ANDROID_NDK=$ANDROID_HOME/ndk/25.2.9519653  # or your NDK version

# Configure build environment
./build_android_config.sh
```

### Build Command
```bash
# Navigate to executorch source
cd ../executorch

# Build Android AAR with all backends
./scripts/build_android_library.sh

# For specific architectures only:
export ANDROID_ABIS=("arm64-v8a")  # or ("arm64-v8a" "x86_64")
./scripts/build_android_library.sh
```

### Optional: Hardware-Specific Builds

#### Qualcomm Devices (QNN Backend)
```bash
# Download Qualcomm Neural Processing SDK
export QNN_SDK_ROOT=/path/to/qnn/sdk
./scripts/build_android_library.sh
```

#### MediaTek Devices (Neuron Backend)
```bash
# Set MediaTek NeuroPilot library paths
export NEURON_BUFFER_ALLOCATOR_LIB=/path/to/libneuron_buffer_allocator.so
export NEURON_USDK_ADAPTER_LIB=/path/to/libneuron_usdk_adapter.so
./scripts/build_android_library.sh
```

## Windows Builds

### Supported Backends
- ✅ **XNNPACK** - High-performance neural network operators
- ✅ **Optimized Kernels** - CPU-optimized implementations

### Prerequisites
- **Visual Studio 2019+** with C++ workload
- **CMake 3.24+**
- **Git for Windows**

### Build Command
```bash
# From executorch_builder directory
cd sdk/corecpp/third_party/executorch_builder
make -f Makefile.platforms windows

# Manual build:
cd ../executorch
mkdir build-windows
cd build-windows
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DEXECUTORCH_BUILD_XNNPACK=ON \
    -DEXECUTORCH_BUILD_EXTENSION_DATA_LOADER=ON \
    -DEXECUTORCH_BUILD_EXTENSION_MODULE=ON \
    -DEXECUTORCH_BUILD_EXTENSION_TENSOR=ON \
    -DEXECUTORCH_BUILD_KERNELS_OPTIMIZED=ON \
    -DEXECUTORCH_BUILD_KERNELS_QUANTIZED=ON \
    -G "Visual Studio 16 2019" \
    -A x64
cmake --build . --config Release
```

## Linux Builds

### Supported Backends
- ✅ **XNNPACK** - High-performance neural network operators
- ✅ **Optimized Kernels** - CPU-optimized implementations

### Prerequisites
- **GCC 8+ or Clang 8+**
- **CMake 3.24+**
- **Python 3.8+**

### Build Command
```bash
# From executorch_builder directory
cd sdk/corecpp/third_party/executorch_builder
make -f Makefile.platforms linux

# Manual build:
cd ../executorch
mkdir build-linux
cd build-linux
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DEXECUTORCH_BUILD_XNNPACK=ON \
    -DEXECUTORCH_BUILD_EXTENSION_DATA_LOADER=ON \
    -DEXECUTORCH_BUILD_EXTENSION_MODULE=ON \
    -DEXECUTORCH_BUILD_EXTENSION_TENSOR=ON \
    -DEXECUTORCH_BUILD_KERNELS_OPTIMIZED=ON \
    -DEXECUTORCH_BUILD_KERNELS_QUANTIZED=ON
make -j$(nproc)
```

## Testing the Build

### Build executor_runner
```bash
# Navigate to executorch source
cd sdk/corecpp/third_party/executorch

# Create test build
mkdir cmake-out && cd cmake-out
cmake .. \
    -DEXECUTORCH_BUILD_EXECUTOR_RUNNER=ON \
    -DEXECUTORCH_BUILD_GFLAGS=ON \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_BUILD_TYPE=Release
make -j$(nproc) executor_runner
```

### Test with Sample Model
```bash
# Test with the e5 embedding model
./executor_runner --model_path ../../models/embedding/generated_models/e5_complete_static_input_512_attention_512.pte

# Expected output: 384-dimensional tensor with float values
```

### Build Custom Test Runner
```bash
# From executorch_builder directory
cd sdk/corecpp/third_party/executorch_builder

# Build the simple test runner
mkdir test-build && cd test-build
cmake -f ../CMakeLists_test.txt .. && make test_model

# Run test
./test_model ../executorch/models/embedding/generated_models/e5_complete_static_input_512_attention_512.pte
```

## Using the Prebuilt Libraries

### Linux and Windows (CMake Projects)

#### Header Locations
```bash
# Main runtime headers
sdk/corecpp/third_party/executorch/runtime/

# Generated schema headers  
sdk/corecpp/third_party/executorch/cmake-out/schema/include/

# Kernel headers (optional)
sdk/corecpp/third_party/executorch/kernels/
```

#### Static Library Locations
```bash
# Linux build output
sdk/corecpp/third_party/executorch/build-linux/

# Windows build output  
sdk/corecpp/third_party/executorch/build-windows/

# Test build output (all platforms)
sdk/corecpp/third_party/executorch/cmake-out/
```

#### CMake Integration
```cmake
cmake_minimum_required(VERSION 3.24)
project(my_executorch_app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)

# Set executorch path
set(EXECUTORCH_DIR "${CMAKE_CURRENT_SOURCE_DIR}/path/to/executorch")

# Create your executable
add_executable(my_app main.cpp)

# Include directories
target_include_directories(my_app PRIVATE
    ${EXECUTORCH_DIR}                                    # Main source headers
    ${EXECUTORCH_DIR}/cmake-out/schema/include           # Generated schema
    ${EXECUTORCH_DIR}/cmake-out/kernels/portable         # Kernel headers (optional)
)

# Link libraries (adjust paths for your platform)
target_link_libraries(my_app PRIVATE
    ${EXECUTORCH_DIR}/cmake-out/libexecutorch.a          # Core runtime
    ${EXECUTORCH_DIR}/cmake-out/libexecutorch_core.a     # Core functionality
    ${EXECUTORCH_DIR}/cmake-out/kernels/portable/libportable_kernels.a  # Portable kernels
    ${EXECUTORCH_DIR}/cmake-out/kernels/optimized/liboptimized_kernels.a # Optimized kernels
)

# Required definitions
target_compile_definitions(my_app PRIVATE
    C10_USING_CUSTOM_GENERATED_MACROS
    ET_LOG_ENABLED=1
    ET_MIN_LOG_LEVEL=Info
)
```

#### Basic Usage Example
```cpp
#include "executorch/runtime/executor/program.h"
#include "executorch/runtime/executor/method.h"
#include "executorch/runtime/core/error.h"
#include "executorch/schema/program_generated.h"

#include <iostream>
#include <fstream>
#include <vector>

using namespace torch::executor;

int main() {
    // Load model file
    std::ifstream file("model.pte", std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open model file" << std::endl;
        return 1;
    }
    
    // Read file contents
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    file.close();
    
    // Create program
    Result<Program> program = Program::load(data.data(), data.size());
    if (!program.ok()) {
        std::cerr << "Failed to load program" << std::endl;
        return 1;
    }
    
    // Get method
    Result<Method> method = program->get_method("forward");
    if (!method.ok()) {
        std::cerr << "Failed to get method" << std::endl;
        return 1;
    }
    
    // Execute method
    Error status = method->execute();
    if (status != Error::Ok) {
        std::cerr << "Execution failed" << std::endl;
        return 1;
    }
    
    std::cout << "Model executed successfully!" << std::endl;
    return 0;
}
```

### macOS and iOS (Xcode Projects)

#### XCFramework Locations
```bash
# All XCFrameworks are located in:
sdk/corecpp/third_party/executorch/apple-frameworks/

# Available frameworks:
# - executorch.xcframework           (Core runtime)
# - backend_coreml.xcframework       (CoreML backend)
# - backend_mps.xcframework          (Metal Performance Shaders)
# - backend_xnnpack.xcframework      (XNNPACK backend)
# - kernels_portable.xcframework     (Portable CPU kernels)
# - kernels_optimized.xcframework    (Optimized CPU kernels)
```

#### Xcode Integration
1. **Add Frameworks to Project:**
   - Drag and drop all `.xcframework` files into your Xcode project
   - In target settings → "Frameworks, Libraries, and Embedded Content"
   - Set each framework to "Embed & Sign"

2. **Build Settings:**
   - Set "C++ Language Dialect" to "C++17"
   - Set "iOS Deployment Target" to "15.0+"
   - Set "macOS Deployment Target" to "10.15+"

3. **Preprocessor Macros:**
   ```
   C10_USING_CUSTOM_GENERATED_MACROS=1
   ET_LOG_ENABLED=1
   ET_MIN_LOG_LEVEL=Info
   ```

#### Swift Integration Example
```swift
import executorch
import backend_xnnpack  // Import needed backends

class ModelRunner {
    private var program: Program?
    private var method: Method?
    
    func loadModel(from path: String) -> Bool {
        guard let data = NSData(contentsOfFile: path) else {
            print("Failed to load model file")
            return false
        }
        
        let result = Program.load(data.bytes, size: data.length)
        guard result.ok else {
            print("Failed to create program")
            return false
        }
        
        program = result.value
        
        let methodResult = program?.getMethod("forward")
        guard methodResult?.ok == true else {
            print("Failed to get method")
            return false
        }
        
        method = methodResult?.value
        return true
    }
    
    func execute() -> Bool {
        guard let method = method else { return false }
        
        let status = method.execute()
        return status == .Ok
    }
}
```

#### Objective-C Integration Example
```objc
#import <executorch/executorch.h>
#import <backend_xnnpack/backend_xnnpack.h>

@interface ModelRunner : NSObject
- (BOOL)loadModelFromPath:(NSString *)path;
- (BOOL)execute;
@end

@implementation ModelRunner {
    torch::executor::Program *_program;
    torch::executor::Method *_method;
}

- (BOOL)loadModelFromPath:(NSString *)path {
    NSData *data = [NSData dataWithContentsOfFile:path];
    if (!data) {
        NSLog(@"Failed to load model file");
        return NO;
    }
    
    auto result = torch::executor::Program::load(data.bytes, data.length);
    if (!result.ok()) {
        NSLog(@"Failed to create program");
        return NO;
    }
    
    _program = new torch::executor::Program(std::move(result.get()));
    
    auto methodResult = _program->get_method("forward");
    if (!methodResult.ok()) {
        NSLog(@"Failed to get method");
        return NO;
    }
    
    _method = new torch::executor::Method(std::move(methodResult.get()));
    return YES;
}

- (BOOL)execute {
    if (!_method) return NO;
    
    auto status = _method->execute();
    return status == torch::executor::Error::Ok;
}

@end
```

### Android (Gradle Projects)

#### AAR Location
```bash
# Android AAR is generated at:
sdk/corecpp/third_party/executorch/executorch.aar
```

#### Gradle Integration
```gradle
// In your app/build.gradle
android {
    compileSdk 34
    
    defaultConfig {
        minSdk 26
        targetSdk 34
    }
    
    compileOptions {
        sourceCompatibility JavaVersion.VERSION_1_8
        targetCompatibility JavaVersion.VERSION_1_8
    }
    
    ndkVersion "25.2.9519653"
}

dependencies {
    implementation files('path/to/executorch.aar')
    
    // If using specific backends, you may need additional dependencies
    // implementation 'com.qualcomm.qti:qnn:1.0.0'      // For QNN backend
    // implementation 'com.mediatek:neuron:1.0.0'       // For Neuron backend
}
```

#### Java Integration Example
```java
import org.pytorch.executorch.EValue;
import org.pytorch.executorch.Module;
import org.pytorch.executorch.Tensor;

public class ModelRunner {
    private Module module;
    
    public boolean loadModel(String modelPath) {
        try {
            module = Module.load(modelPath);
            return true;
        } catch (Exception e) {
            Log.e("ModelRunner", "Failed to load model", e);
            return false;
        }
    }
    
    public Tensor execute(Tensor input) {
        if (module == null) return null;
        
        try {
            EValue[] inputs = {EValue.from(input)};
            EValue output = module.execute("forward", inputs);
            return output.toTensor();
        } catch (Exception e) {
            Log.e("ModelRunner", "Execution failed", e);
            return null;
        }
    }
}
```

#### Kotlin Integration Example
```kotlin
import org.pytorch.executorch.EValue
import org.pytorch.executorch.Module
import org.pytorch.executorch.Tensor

class ModelRunner {
    private var module: Module? = null
    
    fun loadModel(modelPath: String): Boolean {
        return try {
            module = Module.load(modelPath)
            true
        } catch (e: Exception) {
            Log.e("ModelRunner", "Failed to load model", e)
            false
        }
    }
    
    fun execute(input: Tensor): Tensor? {
        val module = this.module ?: return null
        
        return try {
            val inputs = arrayOf(EValue.from(input))
            val output = module.execute("forward", inputs)
            output.toTensor()
        } catch (e: Exception) {
            Log.e("ModelRunner", "Execution failed", e)
            null
        }
    }
}
```

## Backend Configuration Summary

| Platform | XNNPACK | CoreML | MPS | Vulkan | QNN | Neuron | Optimized |
|----------|---------|--------|-----|--------|-----|--------|-----------|
| iOS      | ✅      | ✅     | ✅  | ❌     | ❌  | ❌     | ✅        |
| macOS    | ✅      | ✅     | ✅  | ❌     | ❌  | ❌     | ✅        |
| Android  | ✅      | ❌     | ❌  | ✅     | ✅* | ✅*    | ❌        |
| Windows  | ✅      | ❌     | ❌  | ❌     | ❌  | ❌     | ✅        |
| Linux    | ✅      | ❌     | ❌  | ❌     | ❌  | ❌     | ✅        |

*\* Requires vendor-specific SDKs*

## Troubleshooting

### Common Issues

#### 1. gflags CMake Version Error
```bash
# Add this flag to cmake commands:
-DCMAKE_POLICY_VERSION_MINIMUM=3.5
```

#### 2. Missing Submodules
```bash
cd sdk/corecpp/third_party/executorch
git submodule sync
git submodule update --init --recursive
```

#### 3. Python Module Not Found (zstd)
```bash
pip3 install zstd
```

#### 4. Buck2 Download Issues
```bash
cd sdk/corecpp/third_party/executorch
# Clean and retry
./install_executorch.sh --clean
rm -rf buck2-bin/
# Re-run build command
```

#### 5. iOS Toolchain Not Found
```bash
# Ensure Xcode command line tools are installed
xcode-select --install

# Verify toolchain exists
ls sdk/corecpp/third_party/executorch/third-party/ios-cmake/ios.toolchain.cmake
```

#### 6. Xcode Framework Not Found
```bash
# Ensure all XCFrameworks are added to your target
# Check "Frameworks, Libraries, and Embedded Content" in target settings
# Set each framework to "Embed & Sign"
```

### Build Output Locations

#### iOS/macOS
```
executorch/apple-frameworks/
├── executorch.xcframework
├── backend_coreml.xcframework
├── backend_mps.xcframework
├── backend_xnnpack.xcframework
├── kernels_portable.xcframework
└── kernels_optimized.xcframework
```

#### Android
```
executorch/executorch.aar  # Contains all architectures and backends
```

#### Windows/Linux
```
executorch/build-windows/  # or build-linux/
├── libexecutorch.a
├── libexecutorch_core.a
├── libportable_ops_lib.a
├── kernels/portable/libportable_kernels.a
└── kernels/optimized/liboptimized_kernels.a
```

## Build System Files

Located in `sdk/corecpp/third_party/executorch_builder/`:

- **`Makefile.platforms`** - Cross-platform build automation
- **`build_android_config.sh`** - Android configuration script
- **`test_model.cpp`** - Simple model test application
- **`CMakeLists_test.txt`** - CMake config for test builds
- **`build_executorch_README.md`** - This documentation

## Performance Optimization Tips

1. **Use Release builds** for production (`-DCMAKE_BUILD_TYPE=Release`)
2. **Enable appropriate backends** for your target hardware
3. **Use XNNPACK** for CPU optimization on all platforms
4. **Use Optimized Kernels** for maximum CPU performance
5. **Use CoreML/MPS** on Apple devices for GPU acceleration
6. **Use Vulkan** on Android for cross-vendor GPU support
7. **Use QNN/Neuron** for NPU acceleration on supported devices

## Integration Notes

### iOS/macOS Projects
- Link all required XCFrameworks (including kernels_optimized.xcframework)
- Set minimum deployment targets (iOS 15+, macOS 10.15+)
- Enable C++17 standard in build settings
- Add required preprocessor macros

### Android Projects
- Add executorch.aar to your module dependencies
- Ensure NDK version matches build environment
- Target API level 26+
- Handle native library loading in your app

### Desktop Applications
- Link static libraries in correct order (include optimized kernels)
- Include headers from the source directory
- Set C++17 standard in compiler flags
- Add required preprocessor definitions

---

*This guide covers building Executorch v0.4+ with backend support optimized for production workloads.* 