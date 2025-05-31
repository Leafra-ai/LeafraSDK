# ICU Library Integration

This document describes the ICU (International Components for Unicode) library integration in the LeafraSDK.

## Overview

ICU provides robust Unicode and locale support for C++ applications. The LeafraSDK integrates ICU to handle:
- Unicode text processing
- Character classification and properties
- Text boundary detection
- Locale-aware operations

## Platform-Specific Implementation

### iOS/macOS
- **iOS**: Uses ICU libraries from the iOS SDK (`icuuc`, `icui18n`, `icudata`)
- **macOS**: Uses Apple's bundled `libicucore` which provides a unified ICU interface
- **Headers**: System headers from `/usr/include/unicode/`
- **Linking**: Automatic detection and linking

### Android
- **API Level 31+**: Uses ICU libraries from the Android NDK
- **Older APIs**: ICU is not available, requires bundling (not implemented)
- **Libraries**: `libicuuc.so`, `libicui18n.so`, `libicudata.so`
- **Headers**: NDK headers

### Windows
- **Libraries**: Bundled prebuilt libraries in `sdk/corecpp/third_party/prebuilt/icu/windows/`
- **Architectures**: x32, x64, ARM64
- **Format**: DLL + import libraries (.lib files)
- **Headers**: Bundled headers

### Linux
- **Detection**: Uses pkg-config first, fallback to find_library
- **Libraries**: System libraries (`libicuuc`, `libicui18n`, `libicudata`)
- **Installation**: Requires `libicu-dev` package
- **Headers**: System headers

## React Native Integration

### iOS
ICU support is included in the React Native iOS bindings through:

- **Podspec Configuration**: Updated `LeafraSDK.podspec` includes:
  - `LEAFRA_HAS_ICU=1` preprocessor definition
  - `libicucore` library linking
  - Automatic inheritance from LeafraCore

- **CMakeLists.txt**: iOS React Native module automatically inherits ICU from LeafraCore

### Android
ICU support for React Native Android:

- **CMakeLists.txt**: Created placeholder configuration in `sdk/react-native/android/CMakeLists.txt`
- **API Level Check**: Validates Android API level for ICU support
- **Inheritance**: ICU support automatically inherited from LeafraCore when implementation is added

### Windows  
ICU support for React Native Windows:

- **CMakeLists.txt**: Created placeholder configuration in `sdk/react-native/windows/CMakeLists.txt`
- **Architecture Detection**: Automatically detects Windows architecture (x32, x64, ARM64)
- **Path Validation**: Checks for ICU library availability for the target architecture
- **Inheritance**: ICU support automatically inherited from LeafraCore when implementation is added

**Note**: Android and Windows React Native implementations are currently placeholders. The ICU integration is ready, but actual React Native module source files need to be created.

## Build Integration

### CMake Configuration

The ICU integration is handled in `sdk/CMakeLists.txt` with platform-specific detection:

```cmake
# ICU Integration section
set(ICU_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/corecpp/third_party/prebuilt/icu")

# Platform-specific configuration...
```

### Preprocessor Definitions

When ICU is available, the following preprocessor definition is set:
```cpp
#define LEAFRA_HAS_ICU 1
```

### Linking

ICU is linked to the LeafraCore library in `sdk/corecpp/CMakeLists.txt`:

```cmake
# Link ICU if available
if(TARGET ICU::uc AND TARGET ICU::i18n)
    target_link_libraries(LeafraCore PRIVATE ICU::i18n ICU::uc)
    if(TARGET ICU::data)
        target_link_libraries(LeafraCore PRIVATE ICU::data)
    endif()
    target_compile_definitions(LeafraCore PRIVATE LEAFRA_HAS_ICU=1)
endif()
```

## Usage in Code

### Conditional Compilation

```cpp
#ifdef LEAFRA_HAS_ICU
#include <unicode/uchar.h>
#include <unicode/utypes.h>

void processUnicodeText() {
    // ICU-based Unicode processing
}
#else
void processUnicodeText() {
    // Fallback implementation
}
#endif
```

### Available Headers by Platform

#### macOS (libicucore)
- `unicode/uchar.h` - Character properties
- `unicode/utypes.h` - Basic types
- `unicode/ptypes.h` - Platform types
- Limited set compared to full ICU

#### iOS/Android/Linux/Windows (Full ICU)
- `unicode/unistr.h` - UnicodeString class
- `unicode/brkiter.h` - Break iterators
- `unicode/uchar.h` - Character properties
- `unicode/utypes.h` - Basic types
- And many more...

## Build Status Messages

The build system provides clear status messages:

- ✅ ICU integration enabled (system libraries)
- ✅ ICU integration enabled (macOS ICU Core)
- ✅ ICU integration enabled (Windows x64)
- ⚠️ Building LeafraCore without ICU
- ❌ ICU libraries not found

### React Native Specific Messages

- 📱 Android React Native bindings configured (placeholder)
- 🖥️ Windows React Native bindings configured (placeholder)
- ✅ Android API XX supports ICU
- ⚠️ Android API XX - ICU support limited

## Dependencies

### Linux Installation
```bash
# Ubuntu/Debian
sudo apt-get install libicu-dev

# CentOS/RHEL
sudo yum install libicu-devel

# Fedora
sudo dnf install libicu-devel
```

### Windows
Prebuilt libraries are included in the repository at:
- `sdk/corecpp/third_party/prebuilt/icu/windows/win-x32/`
- `sdk/corecpp/third_party/prebuilt/icu/windows/win-x64/`
- `sdk/corecpp/third_party/prebuilt/icu/windows/win-arm64/`

### iOS/macOS
ICU is provided by the platform SDK - no additional installation required.

### Android
ICU is available in NDK for API level 31+. For older API levels, ICU would need to be bundled (not currently implemented).

## Testing

The integration can be tested by building the SDK:

```bash
cd example/sdkcmdapp/build
make clean && make -j4
```

Look for ICU status messages in the build output. 