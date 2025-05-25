# LeafraSDK - Cross-Platform C++ SDK

A high-performance, cross-platform C++ SDK with React Native bindings for iOS, macOS, Android, and Windows.

## 🏗️ Architecture

```
LeafraSDK/
├── corecpp/                    # Core C++ library
│   ├── include/leafra/         # Public headers
│   │   ├── leafra_core.h      # Main SDK interface
│   │   ├── types.h            # Common types and structures
│   │   ├── data_processor.h   # Data processing utilities
│   │   ├── math_utils.h       # Mathematical operations
│   │   └── platform_utils.h   # Platform-specific utilities
│   ├── src/                   # Implementation files
│   └── CMakeLists.txt         # Core build configuration
├── ios/                       # iOS/macOS React Native bindings
├── android/                   # Android React Native bindings
├── windows/                   # Windows bindings
├── react-native/              # React Native TypeScript interface
└── CMakeLists.txt             # Main build configuration
```

## 🚀 Features

- **Cross-Platform**: iOS, macOS, Android, Windows, Linux support
- **High Performance**: Optimized C++ core with minimal overhead
- **React Native Integration**: Seamless JavaScript/TypeScript bindings
- **Modern C++17**: Uses latest C++ features and best practices
- **CMake Build System**: Easy cross-platform compilation
- **Thread-Safe**: All operations are thread-safe by default
- **Memory Safe**: RAII patterns and smart pointers throughout

## 📋 Requirements

### General
- CMake 3.18 or later
- C++17 compatible compiler
- Git

### iOS/macOS
- Xcode 12 or later
- iOS 15.1+ / macOS 12.0+
- CocoaPods

### Android
- Android NDK 21+
- Android API Level 23+

### Windows
- Visual Studio 2019 or later
- Windows 10 SDK

## 🛠️ Building the SDK

### Quick Start
```bash
# Build for current platform
cd sdk
./build.sh

# Build for iOS
./build.sh --ios

# Build for macOS  
./build.sh --macos

# Build for Android (requires Android NDK)
./build.sh --android

# Clean build
./build.sh --clean

# Debug build
./build.sh --debug

# Verbose output
./build.sh --verbose
```

### Manual CMake Build
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
make install
```

### iOS Specific
```bash
# Build for iOS device
./build.sh --ios

# Build for iOS simulator
./build.sh --ios --simulator

# Or manually with cmake
mkdir build-ios && cd build-ios
cmake .. -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphoneos
make -j$(nproc)
```

## 📱 React Native Integration

### Installation in React Native Project

1. **Install the package:**
```bash
cd your-react-native-project
npm install file:../path/to/LeafraSDK/sdk/react-native
```

2. **iOS Setup:**
```bash
cd ios && pod install
```

3. **Link native dependencies:**
```bash
npx react-native link react-native-leafra-sdk
```

### Usage in React Native

```typescript
import LeafraSDK, { LeafraConfig, ResultCode, Point2D } from 'react-native-leafra-sdk';

// Initialize SDK
const config: LeafraConfig = {
  name: 'MyApp',
  version: '1.0.0',
  debugMode: true,
  maxThreads: 4,
  bufferSize: 1024
};

const result = await LeafraSDK.initialize(config);
if (result === ResultCode.SUCCESS) {
  console.log('SDK initialized successfully');
}

// Process data
const inputData = [1, 2, 3, 4, 5];
const processResult = await LeafraSDK.processData(inputData);
console.log('Processed data:', processResult.output);

// Calculate distance between points
const p1: Point2D = { x: 0, y: 0 };
const p2: Point2D = { x: 3, y: 4 };
const distance = await LeafraSDK.calculateDistance2D(p1, p2);
console.log('Distance:', distance); // Should be 5.0

// Set event callback
LeafraSDK.setEventCallback((message: string) => {
  console.log('SDK Event:', message);
});

// Cleanup
await LeafraSDK.shutdown();
```

## 🧪 Core C++ API

### Basic Usage

```cpp
#include "leafra/leafra_core.h"

using namespace leafra;

// Create SDK instance
auto sdk = LeafraCore::create();

// Configure SDK
Config config;
config.name = "MyApp";
config.version = "1.0.0";
config.debug_mode = true;

// Initialize
auto result = sdk->initialize(config);
if (result == ResultCode::SUCCESS) {
    std::cout << "SDK initialized successfully" << std::endl;
}

// Process data
data_buffer_t input = {1, 2, 3, 4, 5};
data_buffer_t output;
result = sdk->process_data(input, output);

// Set event callback
sdk->set_event_callback([](const std::string& message) {
    std::cout << "Event: " << message << std::endl;
});

// Cleanup
sdk->shutdown();
```

### Mathematical Operations

```cpp
#include "leafra/math_utils.h"

using namespace leafra;

MathUtils math;
math.initialize();

// 2D distance
Point2D p1{0, 0};
Point2D p2{3, 4};
double dist = math.distance(p1, p2); // Result: 5.0

// Matrix operations
Matrix3x3 a, b, result;
// ... populate matrices ...
auto code = math.multiply(a, b, result);
double det = math.determinant(a);
```

### Data Processing

```cpp
#include "leafra/data_processor.h"

using namespace leafra;

DataProcessor processor;
processor.initialize();
processor.set_buffer_size(2048);

data_buffer_t input = {/* your data */};
data_buffer_t output;
auto result = processor.process(input, output);
```

## 🔧 Build Configuration Options

### CMake Options

```bash
# Core options
-DLEAFRA_BUILD_SHARED=ON|OFF          # Build shared library (default: ON)
-DLEAFRA_BUILD_TESTS=ON|OFF           # Build tests (default: OFF)
-DLEAFRA_BUILD_EXAMPLES=ON|OFF        # Build examples (default: OFF)
-DLEAFRA_BUILD_RN_BINDINGS=ON|OFF     # Build React Native bindings (default: ON)

# Platform specific
-DCMAKE_SYSTEM_NAME=iOS               # Target iOS
-DCMAKE_OSX_SYSROOT=iphoneos         # iOS device SDK
-DCMAKE_OSX_SYSROOT=iphonesimulator  # iOS simulator SDK
-DCMAKE_OSX_ARCHITECTURES="arm64"     # Target architecture
```

### Xcode Integration

To integrate with an existing Xcode project:

1. **Add the framework:**
```bash
# In your iOS project's Podfile
pod 'LeafraSDK', :path => '../path/to/LeafraSDK/sdk'
```

2. **Or manually link:**
- Add `LeafraCore.framework` to your project
- Add to "Frameworks, Libraries, and Embedded Content"
- Set "Embed & Sign" for distribution

## 🧩 Integration with Leafra App

The Leafra React Native app is pre-configured to use this SDK:

### File Structure in Leafra App
```
example/Leafra/
├── App.tsx                    # Main app (can import SDK)
├── metro.config.js            # Configured to watch SDK files
├── react-native.config.js     # Links SDK automatically
└── ios/
    └── Leafra/
        └── CMakeLists.txt     # Links with LeafraCore
```

### Usage in Leafra App

```typescript
// In App.tsx
import LeafraSDK from 'react-native-leafra-sdk';

// ... in your component
useEffect(() => {
  const initSDK = async () => {
    const result = await LeafraSDK.initialize({
      name: 'Leafra',
      version: '1.0.0',
      debugMode: __DEV__,
    });
    
    if (result === ResultCode.SUCCESS) {
      console.log('LeafraSDK ready!');
    }
  };
  
  initSDK();
}, []);
```

## 🔍 Debugging

### Enable Debug Output
```cpp
Config config;
config.debug_mode = true;  // Enables console logging
```

```typescript
// In React Native
const config: LeafraConfig = {
  debugMode: true  // Enables debug logs
};
```

### Verbose CMake Build
```bash
./build.sh --verbose
# or
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
```

### Xcode Debugging
1. Open `ios/Leafra.xcworkspace` in Xcode
2. Set breakpoints in C++ code
3. Build and run with debugger attached

## 📚 API Reference

### Core Classes

#### `LeafraCore`
- `initialize(config)` - Initialize SDK
- `shutdown()` - Cleanup resources
- `process_data(input, output)` - Process data
- `set_event_callback(callback)` - Set event handler

#### `DataProcessor`  
- `process(input, output)` - Transform data
- `set_buffer_size(size)` - Configure buffer

#### `MathUtils`
- `distance(p1, p2)` - Calculate distances
- `multiply(a, b, result)` - Matrix multiplication
- `determinant(matrix)` - Matrix determinant

### Result Codes
- `SUCCESS` (0) - Operation succeeded
- `ERROR_INVALID_PARAMETER` (-1) - Invalid input
- `ERROR_INITIALIZATION_FAILED` (-2) - Init failed
- `ERROR_PROCESSING_FAILED` (-3) - Processing error
- `ERROR_NOT_IMPLEMENTED` (-4) - Feature not available
- `ERROR_OUT_OF_MEMORY` (-5) - Memory allocation failed

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test on all target platforms
5. Submit a pull request

## 📄 License

MIT License - see LICENSE file for details 