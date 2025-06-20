# LlamaCpp Integration for LeafraSDK

This directory contains LlamaCpp prebuilt libraries and integration for the LeafraSDK.

## Directory Structure

```
llamacpp/
├── apple/
│   └── llama.xcframework/          # iOS/macOS XCFramework (includes headers)
├── android/
│   ├── arm64-v8a/                  # Android ARM64 libraries + headers
│   ├── armeabi-v7a/               # Android ARMv7 libraries + headers
│   ├── x86_64/                    # Android x86_64 libraries + headers
│   └── x86/                       # Android x86 libraries + headers
├── windows/
│   ├── x64/                       # Windows 64-bit libraries + headers
│   └── x86/                       # Windows 32-bit libraries + headers
└── linux/
    ├── x86_64/                    # Linux x86_64 libraries + headers
    └── aarch64/                   # Linux ARM64 libraries + headers
```

## Current Status

### ✅ Supported Platforms
- **iOS** (Device + Simulator): Using XCFramework
- **macOS**: Using XCFramework

### 🔄 Platforms Needing Libraries
- **Android**: Place `libllama.so` in respective ABI directories
- **Windows**: Place `llama.dll` and `llama.lib` in architecture directories
- **Linux**: Place `libllama.so` in architecture directories

## Integration Details

### Build System Integration
The LlamaCpp integration is automatically handled by the CMake build system:

1. **Library Detection**: CMake automatically detects available libraries per platform
2. **Header Configuration**: Headers are sourced from the XCFramework for Apple platforms, and from platform-specific `include/` directories for others
3. **Linking**: Libraries are linked as `LlamaCpp::LlamaCpp` target
4. **Preprocessor Define**: `LEAFRA_HAS_LLAMACPP=1` is set when libraries are found

### Usage in Code
```cpp
#ifdef LEAFRA_HAS_LLAMACPP
#include "leafra/leafra_llamacpp.h"

// Use LlamaCpp functionality
leafra::llamacpp::initialize_llamacpp();
// ... your code ...
leafra::llamacpp::cleanup_llamacpp();
#endif
```

## Adding Libraries for Additional Platforms

### Android
Place the following files in their respective ABI directories:
- `android/arm64-v8a/libllama.so` + headers in `android/arm64-v8a/include/`
- `android/armeabi-v7a/libllama.so` + headers in `android/armeabi-v7a/include/`
- `android/x86_64/libllama.so` + headers in `android/x86_64/include/`
- `android/x86/libllama.so` + headers in `android/x86/include/`

### Windows
Place the following files in their respective architecture directories:
- `windows/x64/llama.dll` + `windows/x64/llama.lib` + headers in `windows/x64/include/`
- `windows/x86/llama.dll` + `windows/x86/llama.lib` + headers in `windows/x86/include/`

### Linux
Place the following files in their respective architecture directories:
- `linux/x86_64/libllama.so` + headers in `linux/x86_64/include/`
- `linux/aarch64/libllama.so` + headers in `linux/aarch64/include/`

## Building Instructions

### iOS
```bash
./build.sh --ios --embedding-fw=coreml
```

### macOS
```bash
./build.sh --macos --embedding-fw=coreml
```

### Android (when libraries are available)
```bash
./build.sh --android --embedding-fw=coreml
```

## Notes

- The current integration provides stubs for compilation when headers are problematic
- Full LlamaCpp functionality will be available once library/header issues are resolved
- The build system automatically detects available libraries and only links when found
- Missing libraries result in warnings, not build failures

## Version Information

- LlamaCpp version: Based on commit b5699 (see llama.cpp-b5699.README)
- XCFramework built for iOS 12.0+ and macOS 10.15+
- Headers are compatible across all platforms 