# LeafraSDK - Cross-Platform SDK

LeafraSDK is a high-performance, cross-platform SDK that provides mathematical operations, data processing, and platform utilities. It offers both **native** and **React Native** integration options for maximum flexibility.

## 🏗️ Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    LeafraSDK Core (C++)                     │
│                     corecpp/                                │
│  ┌─────────────────┬─────────────────┬─────────────────┐    │
│  │   LeafraCore    │   MathUtils     │ DataProcessor   │    │
│  │   Platform      │   Distance      │   Algorithms    │    │
│  │   Management    │   Matrix Ops    │   Filtering     │    │
│  └─────────────────┴─────────────────┴─────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Integration Layer                        │
├─────────────────────────────┬───────────────────────────────┤
│         Native              │        React Native           │
│                             │                               │
│  ┌─────────────────────┐    │  ┌─────────────────────────┐  │
│  │ iOS (Swift/ObjC)    │    │  │ iOS (Objective-C++)     │  │
│  │ macOS (Swift/ObjC)  │    │  │ macOS (Objective-C++)   │  │
│  │ Android (Kotlin)    │    │  │ Android (Java/JNI)      │  │
│  │ Windows (C#/.NET)   │    │  │ Windows (C++/WinRT)     │  │
│  └─────────────────────┘    │  └─────────────────────────┘  │
└─────────────────────────────┴───────────────────────────────┘
```

## 📁 Folder Structure

```
sdk/
├── corecpp/                    # Core C++ SDK implementation
│   ├── include/               # Public headers
│   ├── src/                   # Implementation files
│   └── CMakeLists.txt         # Build configuration
│
├── native/                    # Native platform integrations
│   ├── ios/                   # Swift/Objective-C wrapper
│   ├── macos/                 # macOS-specific (shares iOS)
│   ├── android/               # Kotlin/Java + JNI wrapper
│   └── windows/               # C#/.NET + P/Invoke wrapper
│
├── react-native/              # React Native integrations
│   ├── common/                # Shared TypeScript interfaces
│   │   └── src/               # Common RN types and interfaces
│   ├── ios/                   # iOS RN bridge (Objective-C++)
│   ├── macos/                 # macOS RN bridge (shares iOS)
│   ├── android/               # Android RN bridge (Java/JNI)
│   ├── windows/               # Windows RN bridge (C++/WinRT)
│   ├── package.json           # NPM package configuration
│   └── tsconfig.json          # TypeScript configuration
│
├── build.sh                   # Cross-platform build script
├── LeafraSDK.podspec          # CocoaPods specification
└── README.md                  # This file
```

## 🚀 Quick Start

### Choose Your Integration Path

#### Option 1: Native Integration
Direct integration into native apps without React Native:

```bash
# iOS/macOS (Swift)
# See: native/ios/README.md
pod 'LeafraSDK'

# Android (Kotlin)  
# See: native/android/README.md
implementation 'com.leafra:sdk:1.0.0'

# Windows (C#/.NET)
# See: native/windows/README.md
<PackageReference Include="LeafraSDK" Version="1.0.0" />
```

#### Option 2: React Native Integration
Integration for React Native apps:

```bash
# Install React Native package
npm install react-native-leafra-sdk

# iOS
cd ios && pod install

# Android
# Automatic linking via autolinking
```

## 📱 Platform Support

| Platform | Native Integration | React Native Integration | Status |
|----------|-------------------|-------------------------|---------|
| **iOS** | ✅ Swift/Objective-C | ✅ Objective-C++ Bridge | Implemented |
| **macOS** | ✅ Swift/Objective-C | ✅ Objective-C++ Bridge | Implemented |
| **Android** | 🚧 Kotlin/Java + JNI | 🚧 Java/JNI Bridge | Planned |
| **Windows** | 🚧 C#/.NET + P/Invoke | 🚧 C++/WinRT Bridge | Planned |

### Minimum Requirements

- **iOS**: 15.1+
- **macOS**: 12.0+
- **Android**: API 23+ (Android 6.0)
- **Windows**: Windows 10+
- **React Native**: 0.70+

## 🔧 Core Features

### Mathematical Operations
- 2D/3D distance calculations
- Matrix operations (3x3 multiplication, determinant, inversion)
- Point transformations and interpolation
- Vector mathematics

### Data Processing
- Multiple processing algorithms (transform, reverse, accumulate, filter)
- Configurable processing options
- High-performance batch operations
- Memory-efficient streaming

### Platform Utilities
- Platform detection (iOS, macOS, Android, Windows, Linux)
- Architecture detection (ARM64, x86_64, etc.)
- Mobile platform identification
- System information gathering

### SDK Management
- Initialization and shutdown lifecycle
- Configuration management
- Event callback system
- Thread-safe operations
- Error handling with detailed result codes

## 🛠️ Development

### Building the SDK

The build system uses a unified `build.sh` script with platform-specific flags and intelligent dependency management.

#### Basic Build Commands

```bash
# Build for current platform (macOS/Linux)
./build.sh

# Build for iOS device (ARM64)
./build.sh --ios

# Build for iOS simulator (Universal binary: x86_64 + ARM64)
# Creates a single framework that works on both Intel and Apple Silicon Macs
./build.sh --ios --simulator

# Build for macOS (Universal: x86_64 + ARM64)
./build.sh --macos

# Build for Android
./build.sh --android
```

#### Build Options

```bash
# Clean builds (removes artifacts before building)
./build.sh --clean                    # Clean all artifacts
./build.sh --clean --ios             # Clean iOS device artifacts only
./build.sh --clean --ios --simulator # Clean iOS simulator artifacts only

# Debug builds
./build.sh --debug --ios             # iOS debug build
./build.sh --debug --macos           # macOS debug build

# Verbose output
./build.sh --verbose --ios           # Show detailed build output

# Build specific targets
./build.sh --ios core               # Build only core C++ library
./build.sh --ios bindings           # Build only React Native bindings
./build.sh --ios all                # Build everything (default)
```

#### Automatic Dependency Management

The build system uses intelligent caching and only builds what's needed:

- **Smart Dependency Checking**: 
  - First checks `corecpp/third_party/prebuilt/` for existing artifacts
  - Only builds from source if prebuilt libraries are missing
  
- **SentencePiece**: 
  - Use `--clean` flag to rebuild the prebuilt artifacts 
  - Checks for platform-specific prebuilt artifacts first
  - Builds universal iOS simulator binaries (x86_64 + ARM64) when needed
  - Automatically cleans intermediate architecture-specific builds
- **Other Dependencies**:
  - **PDFium**: Uses prebuilt libraries (no rebuilding needed)
  - **SQLite**: Uses system libraries (iOS/macOS) or prebuilt
  - **ICU**: Uses system `libicucore` (Apple) or prebuilt
  - **TensorFlow Lite**: Uses prebuilt frameworks, manually built from source
    - iOS device (ARM64): ✅ Supported  
    - iOS simulator (x86_64): ✅ Supported
    - iOS simulator (ARM64): ❌ Not supported (framework limitation) 

#### Build Output Locations

```
sdk/
├── build/                    # Build intermediates
│   ├── ios/                 # iOS device build (ARM64)
│   ├── ios-simulator/       # iOS simulator build (x86_64 + ARM64 universal)
│   └── macos/               # macOS build (x86_64 + ARM64 universal)
│
├── install/                 # Final frameworks/libraries
│   ├── ios/Frameworks/      # iOS frameworks (unified install location)
│   └── macos/Frameworks/    # macOS frameworks
│
└── corecpp/third_party/prebuilt/  # Dependencies
    ├── sentencepiece/       # SentencePiece libraries (platform-specific)
    ├── pdfium/             # PDFium libraries (universal XCFrameworks)
    └── tensorflowlite_from_source/ # TensorFlow Lite (iOS only, partial simulator support)
```

#### Universal Binary Support

**iOS Simulator (`./build.sh --ios --simulator`)**:
- 🎯 **Single command** creates a universal framework with **both architectures**
- 📦 **Output**: One `LeafraCore.framework` containing x86_64 + ARM64 code
- 🖥️ **Intel Macs**: Full functionality including TensorFlow Lite
- 💻 **Apple Silicon Macs**: Full functionality except TensorFlow Lite (graceful fallback)
- 🔄 **Runtime detection**: Code automatically adapts based on host architecture

**Verification**: Use `file` and `lipo` to confirm universal binary:
```bash
file build/ios-simulator/corecpp/LeafraCore.framework/LeafraCore
# Output: Mach-O universal binary with 2 architectures: [x86_64] [arm64]

lipo -info build/ios-simulator/corecpp/LeafraCore.framework/LeafraCore  
# Output: Architectures in the fat file: x86_64 arm64
```

#### Platform-Specific Features

**iOS Device (ARM64):**
- ✅ All dependencies: TensorFlow Lite, SentencePiece, PDFium, SQLite, ICU
- ✅ Machine learning capabilities via TensorFlow Lite
- ✅ Full text processing with SentencePiece training

**iOS Simulator (Universal Binary):**
- ✅ **Single framework** works on both Intel and Apple Silicon Macs
- ✅ **Architectures included**: x86_64 + ARM64 in one universal binary
- ✅ **Core dependencies**: SentencePiece, PDFium, SQLite, ICU (all architectures)
- ⚠️ **TensorFlow Lite availability**:
  - x86_64 Intel Macs: ✅ Full support
  - ARM64 Apple Silicon Macs: ❌ Not supported (framework limitation)
- ✅ **Graceful fallback**: Code runs on all Macs, TensorFlow Lite features automatically disabled on ARM64

**macOS (Universal):**
- ✅ Core dependencies: SentencePiece, PDFium, SQLite, ICU
- ⚠️ TensorFlow Lite planned for future release
- ✅ React Native bindings enabled

### Testing

```bash
# Run C++ tests
cd corecpp && mkdir build && cd build
cmake .. && make && ctest

# Test React Native integration
cd example/Leafra
npm install
npx expo run:ios    # iOS
npx expo run:android # Android
```

## 📖 Documentation

### Platform-Specific Guides

- **[Native iOS Integration](native/ios/README.md)** - Swift/Objective-C integration
- **[Native macOS Integration](native/macos/README.md)** - macOS-specific features
- **[Native Android Integration](native/android/README.md)** - Kotlin/Java integration
- **[Native Windows Integration](native/windows/README.md)** - C#/.NET integration

### React Native Guides

- **[React Native iOS](react-native/ios/README.md)** - iOS RN bridge (✅ Implemented)
- **[React Native macOS](react-native/macos/README.md)** - macOS RN bridge
- **[React Native Android](react-native/android/README.md)** - Android RN bridge (🚧 Planned)
- **[React Native Windows](react-native/windows/README.md)** - Windows RN bridge (🚧 Planned)

### API Reference

```cpp
// C++ Core API
namespace leafra {
    class LeafraCore {
        static std::shared_ptr<LeafraCore> create();
        ResultCode initialize(const Config& config);
        ResultCode shutdown();
        bool is_initialized();
        static std::string get_version();
        static std::string get_platform();
    };
    
    class MathUtils {
        double calculate_distance_2d(const Point2D& p1, const Point2D& p2);
        double calculate_distance_3d(const Point3D& p1, const Point3D& p2);
        Matrix3x3 multiply_matrix_3x3(const Matrix3x3& a, const Matrix3x3& b);
        double matrix_determinant(const Matrix3x3& matrix);
    };
    
    class DataProcessor {
        ProcessDataResult process_data(const std::vector<int>& input, 
                                     const ProcessingOptions& options);
    };
}
```

## 🤝 Contributing

1. **Fork the repository**
2. **Create a feature branch**: `git checkout -b feature/amazing-feature`
3. **Make your changes** following the coding standards
4. **Add tests** for new functionality
5. **Update documentation** as needed
6. **Submit a pull request**

### Coding Standards

- **C++**: Follow Google C++ Style Guide
- **Swift**: Follow Swift API Design Guidelines
- **Kotlin**: Follow Kotlin Coding Conventions
- **C#**: Follow Microsoft C# Coding Conventions
- **TypeScript**: Follow TypeScript ESLint rules

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](../LICENSE) file for details.

## 🆘 Support

- **Issues**: [GitHub Issues](https://github.com/your-org/LeafraSDK/issues)
- **Discussions**: [GitHub Discussions](https://github.com/your-org/LeafraSDK/discussions)
- **Documentation**: [Wiki](https://github.com/your-org/LeafraSDK/wiki)

## 🗺️ Roadmap

### Phase 1: Core Foundation ✅
- [x] C++ core implementation
- [x] iOS/macOS React Native bridge
- [x] Build system and CI/CD

### Phase 2: Native Integrations 🚧
- [ ] iOS/macOS Swift wrapper
- [ ] Android Kotlin wrapper
- [ ] Windows C#/.NET wrapper

### Phase 3: React Native Expansion 🚧
- [ ] Android React Native bridge
- [ ] Windows React Native bridge
- [ ] Cross-platform testing

### Phase 4: Advanced Features 🔮
- [ ] Performance optimizations
- [ ] Advanced mathematical operations
- [ ] Machine learning integration
- [ ] Cloud synchronization

---

**LeafraSDK** - Empowering cross-platform development with high-performance native capabilities. 