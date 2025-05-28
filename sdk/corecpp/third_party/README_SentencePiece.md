# SentencePiece Integration for LeafraSDK

This directory contains the build system for integrating Google's SentencePiece library into LeafraSDK across all supported platforms.

User runs: make ios
     ↓
Makefile.sentencepiece calls: python3 build_sentencepiece.py --platform ios
     ↓
Python script runs: cmake [platform-specific-args] /path/to/sentencepiece/source
     ↓
CMake builds SentencePiece using its own CMakeLists.txt


## Overview

SentencePiece is an unsupervised text tokenizer and detokenizer that implements subword units (BPE and Unigram language models). It's essential for modern NLP applications and neural text generation systems.

## Directory Structure

```
third_party/
├── sentencepiece/              # SentencePiece source code (git submodule/clone)
├── prebuilt/
│   └── sentencepiece/         # Built libraries for all platforms
│       ├── macos/             # macOS universal binary
│       ├── ios-device/        # iOS device (arm64)
│       ├── ios-simulator/     # iOS simulator (arm64 + x86_64)
│       ├── sentencepiece.xcframework/  # iOS XCFramework
│       ├── linux/             # Linux x86_64
│       ├── android-arm64-v8a/ # Android ARM64
│       ├── android-armeabi-v7a/ # Android ARMv7
│       ├── android-x86_64/    # Android x86_64
│       ├── android-x86/       # Android x86
│       └── windows-x64/       # Windows x64
├── build_sentencepiece.py     # Cross-platform build script
├── Makefile.sentencepiece     # Build automation
└── README_SentencePiece.md    # This file
```

## Prerequisites

### All Platforms
- **Python 3.6+** for the build script
- **CMake 3.5+**
- **C++17 compatible compiler**

### Platform-Specific Requirements

#### macOS
```bash
# Install via Homebrew
brew install cmake gperftools
```

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install -y cmake build-essential pkg-config libgoogle-perftools-dev libtcmalloc-minimal4
```

#### Linux (CentOS/RHEL/Fedora)
```bash
# CentOS/RHEL
sudo yum install -y cmake gcc-c++ make pkgconfig gperftools-devel

# Fedora
sudo dnf install -y cmake gcc-c++ make pkgconfig gperftools-devel
```

#### iOS
- **Xcode 12+** with command line tools
- **iOS SDK 15.1+**

#### Android
- **Android NDK r21+**
- Set `ANDROID_NDK_ROOT` environment variable

#### Windows
- **Visual Studio 2019+** with C++ tools
- **CMake** (can be installed via Visual Studio installer)
- **vcpkg** (optional, for gperftools)

## Quick Start

### 1. Get SentencePiece Source Code

```bash
cd sdk/corecpp/third_party

# Option A: Clone the repository
git clone https://github.com/google/sentencepiece.git

# Option B: Add as git submodule (recommended)
git submodule add https://github.com/google/sentencepiece.git sentencepiece
```

### 2. Build for Your Platform

```bash
# Build for current platform (auto-detected)
make -f Makefile.sentencepiece

# Or build for specific platforms
make -f Makefile.sentencepiece macos
make -f Makefile.sentencepiece ios
make -f Makefile.sentencepiece linux
make -f Makefile.sentencepiece android
make -f Makefile.sentencepiece windows
```

### 3. Install Dependencies (if needed)

```bash
# Install dependencies for current platform
make -f Makefile.sentencepiece deps
```

## Detailed Build Instructions

### macOS Build

```bash
# Install dependencies
brew install cmake gperftools

# Build universal binary (arm64 + x86_64)
make -f Makefile.sentencepiece macos

# Output: prebuilt/sentencepiece/macos/
```

**Features:**
- Universal binary supporting both Intel and Apple Silicon
- TCMalloc support for performance optimization
- Static library for easy linking

### iOS Build

```bash
# Build for iOS (requires Xcode)
make -f Makefile.sentencepiece ios

# Output: 
# - prebuilt/sentencepiece/ios-device/
# - prebuilt/sentencepiece/ios-simulator/
# - prebuilt/sentencepiece/sentencepiece.xcframework/
```

**Features:**
- Separate builds for device (arm64) and simulator (arm64 + x86_64)
- XCFramework for easy Xcode integration
- Static libraries (required for App Store)
- No TCMalloc (not available on iOS)

### Linux Build

```bash
# Install dependencies
sudo apt-get install -y cmake build-essential pkg-config libgoogle-perftools-dev

# Build
make -f Makefile.sentencepiece linux

# Output: prebuilt/sentencepiece/linux/
```

**Features:**
- Shared library (.so)
- TCMalloc support for performance
- Position-independent code

### Android Build

```bash
# Set Android NDK path
export ANDROID_NDK_ROOT=/path/to/android-ndk

# Build for all ABIs
make -f Makefile.sentencepiece android

# Output:
# - prebuilt/sentencepiece/android-arm64-v8a/
# - prebuilt/sentencepiece/android-armeabi-v7a/
# - prebuilt/sentencepiece/android-x86_64/
# - prebuilt/sentencepiece/android-x86/
```

**Features:**
- Builds for all major Android ABIs
- Shared libraries (.so)
- Android API level 21+ support
- No TCMalloc (not typically used on Android)

### Windows Build

```bash
# Build (requires Visual Studio)
make -f Makefile.sentencepiece windows

# Output: prebuilt/sentencepiece/windows-x64/
```

**Features:**
- Visual Studio 2019+ support
- DLL + import library
- x64 architecture
- Optional TCMalloc support via vcpkg

## Advanced Usage

### Manual Build with Python Script

```bash
# Build for specific platform
python3 build_sentencepiece.py \
    --source ./sentencepiece \
    --output ./prebuilt/sentencepiece \
    --platform macos

# Available platforms: macos, ios, linux, android, windows
```

### Custom Build Options

Edit `build_sentencepiece.py` to customize:
- Compiler flags
- Optimization levels
- Feature toggles
- Library types (static vs shared)

### TCMalloc Configuration

TCMalloc (Google's high-performance memory allocator) provides 10-40% performance improvement:

- **Enabled by default:** macOS, Linux
- **Disabled by default:** iOS, Android, Windows
- **Optional:** Windows (via vcpkg)

To disable TCMalloc, edit the build script and set `SPM_ENABLE_TCMALLOC=OFF`.

## Integration with LeafraSDK

The built libraries are automatically detected by the CMake build system:

```cmake
# CMakeLists.txt automatically finds and links SentencePiece
if(TARGET SentencePiece::SentencePiece)
    target_link_libraries(LeafraCore PRIVATE SentencePiece::SentencePiece)
    target_compile_definitions(LeafraCore PRIVATE LEAFRA_HAS_SENTENCEPIECE=1)
endif()
```

### Usage in C++ Code

```cpp
#include "leafra/leafra_sentencepiece.h"

// Check if available
if (leafra::sentencepiece_utils::is_available()) {
    // Create tokenizer
    leafra::SentencePieceTokenizer tokenizer;
    
    // Load model
    if (tokenizer.load_model("path/to/model.model")) {
        // Tokenize text
        auto tokens = tokenizer.encode("Hello world!");
        
        // Detokenize
        auto text = tokenizer.decode(tokens);
    }
}
```

## Troubleshooting

### Common Issues

1. **"SentencePiece source not found"**
   ```bash
   # Clone the source code first
   git clone https://github.com/google/sentencepiece.git
   ```

2. **"Android NDK not found"**
   ```bash
   export ANDROID_NDK_ROOT=/path/to/android-ndk
   ```

3. **"CMake not found"**
   ```bash
   # macOS
   brew install cmake
   
   # Linux
   sudo apt-get install cmake
   ```

4. **"gperftools not found"**
   ```bash
   # macOS
   brew install gperftools
   
   # Linux
   sudo apt-get install libgoogle-perftools-dev
   ```

### Build Failures

1. **Clean and rebuild:**
   ```bash
   make -f Makefile.sentencepiece clean
   make -f Makefile.sentencepiece macos
   ```

2. **Check dependencies:**
   ```bash
   make -f Makefile.sentencepiece info
   ```

3. **Manual build with verbose output:**
   ```bash
   python3 build_sentencepiece.py --source ./sentencepiece --output ./prebuilt/sentencepiece --platform macos
   ```

## Performance Notes

### TCMalloc Benefits
- 10-40% performance improvement for tokenization
- Reduced memory fragmentation
- Better multi-threaded performance

### Platform Performance
- **Best:** Linux with TCMalloc
- **Good:** macOS with TCMalloc
- **Moderate:** iOS, Android (no TCMalloc)
- **Variable:** Windows (depends on TCMalloc availability)

## Version Information

- **SentencePiece:** Latest from GitHub (recommended)
- **Minimum CMake:** 3.5
- **C++ Standard:** C++17
- **iOS Deployment Target:** 15.1+
- **Android API Level:** 21+

## Contributing

When adding new platforms or features:

1. Update `build_sentencepiece.py`
2. Update `CMakeLists.txt` platform detection
3. Update this README
4. Test on target platform
5. Update CI/CD if applicable

## License

SentencePiece is licensed under the Apache License 2.0. See the SentencePiece repository for details. 