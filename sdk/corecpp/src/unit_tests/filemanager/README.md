# LeafraSDK File Manager Unit Tests

This directory contains comprehensive unit tests for the LeafraSDK File Manager component, providing cross-platform file operations with proper storage isolation and security features.

## Test Suites

The tests are organized into four comprehensive suites:

1. **`test_filemanager_basic`** - Core file operations (create, delete, exists, info)
2. **`test_filemanager_storage_types`** - Storage isolation (AppStorage vs DocumentStorage)
3. **`test_filemanager_operations`** - Advanced operations (copy, rename, directories)
4. **`test_filemanager_edge_cases`** - Edge cases, security, and Unicode support

**Total Coverage:** 27 tests across all critical functionality

## Platform Testing Status

### ✅ **TESTED & VERIFIED**
- **macOS (Apple Silicon)** - ✅ All 27 tests passing
- **iOS Simulator (iPhone 16 Pro, iOS 18.3)** - ✅ All 27 tests passing

### ⚠️ **READY BUT UNTESTED**
- **iOS Device** - Build system configured, requires Apple Developer account

### ❌ **NOT YET IMPLEMENTED**
- **Windows** - File manager implementation exists but no test build system
- **Linux** - File manager implementation exists but no test build system  
- **Android** - File manager implementation exists but no test build system

> **Note:** The file manager implementation (`leafra_filemanager.cpp`) includes cross-platform code with platform-specific sections for Windows, Linux, and Android, but the unit test build system currently only supports Apple platforms (macOS/iOS).

## Prerequisites

- **macOS:** Xcode 14+ with Command Line Tools
- **iOS:** iOS 13.0+ (required for `std::filesystem` support)
- **CMake:** 3.14 or later

## Building and Running Tests

### macOS Native

#### Build
```bash
# Navigate to the test directory
cd sdk/corecpp/src/unit_tests/filemanager

# Create build directory
mkdir build_macos && cd build_macos

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build all test suites
make -j$(nproc)
```

#### Run Tests
```bash
# Run individual test suites
./test_filemanager_basic
./test_filemanager_storage_types
./test_filemanager_operations
./test_filemanager_edge_cases

# Or run all tests with CTest
ctest --verbose
```

**Expected Output:**
```
=== Test Summary ===
Total tests: 27
Passed: 27
Failed: 0
🎉 All tests passed!
```

### iOS Simulator

#### Build
```bash
# Navigate to the test directory
cd sdk/corecpp/src/unit_tests/filemanager

# Create iOS build directory
mkdir build_ios_arm64 && cd build_ios_arm64

# Configure for iOS Simulator with Xcode generator
cmake .. \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
  -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM="" \
  -G Xcode

# Build individual test suites
xcodebuild -project LeafraFileManagerTests.xcodeproj \
  -scheme test_filemanager_basic \
  -configuration Debug \
  -destination 'platform=iOS Simulator,name=iPhone 16 Pro' \
  build

# Repeat for other test suites:
# test_filemanager_storage_types
# test_filemanager_operations  
# test_filemanager_edge_cases
```

#### Run Tests

**Option 1: Using Running Simulator**
```bash
# List available simulators
xcrun simctl list devices

# Find your running simulator's UDID, then run tests directly
xcrun simctl spawn <SIMULATOR_UDID> \
  /path/to/test_filemanager_basic.app/test_filemanager_basic
```

**Option 2: Launch New Simulator**
```bash
# Boot a simulator
xcrun simctl boot "iPhone 16 Pro"

# Install and run test app
xcrun simctl install <SIMULATOR_UDID> Debug-iphonesimulator/test_filemanager_basic.app
xcrun simctl spawn <SIMULATOR_UDID> \
  /path/to/test_filemanager_basic.app/test_filemanager_basic
```

**iOS Simulator Storage Paths:**
- **AppStorage:** `/Users/.../Library/Developer/CoreSimulator/Devices/<UDID>/data/Library/Application Support/com.leafra.filemanager.tests/`
- **DocumentStorage:** `/Users/.../Library/Developer/CoreSimulator/Devices/<UDID>/data/Documents/`

### iOS Device

#### Prerequisites
- Valid Apple Developer account
- Device connected and trusted
- Proper code signing certificates

#### Build
```bash
# Navigate to the test directory
cd sdk/corecpp/src/unit_tests/filemanager

# Create iOS device build directory
mkdir build_ios_device && cd build_ios_device

# Configure for iOS Device
cmake .. \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
  -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM="YOUR_TEAM_ID" \
  -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY="iPhone Developer" \
  -G Xcode

# Build for connected device
xcodebuild -project LeafraFileManagerTests.xcodeproj \
  -scheme test_filemanager_basic \
  -configuration Debug \
  -destination 'platform=iOS,name=Your Device Name' \
  build
```

#### Deploy and Run
```bash
# Install on device (requires device to be connected)
xcodebuild -project LeafraFileManagerTests.xcodeproj \
  -scheme test_filemanager_basic \
  -configuration Debug \
  -destination 'platform=iOS,name=Your Device Name' \
  install

# Run via device console or Xcode
# Tests will output to device logs accessible via:
# - Xcode → Window → Devices and Simulators → View Device Logs
# - Console.app on macOS
```

## Test Configuration

### Info.plist Settings
The tests use a shared `Info.plist` with the following key settings:
- **Minimum iOS Version:** 13.0
- **Bundle Identifier:** `com.leafra.filemanager.tests`
- **App Type:** Application bundle for iOS compatibility

### CMake Configuration
- **iOS Deployment Target:** 13.0+ (required for `std::filesystem`)
- **Architectures:** arm64 (Apple Silicon native)
- **Frameworks:** Foundation, CoreFoundation
- **Language:** Objective-C++ for iOS platform APIs

## Test Features Verified

### ✅ Core Functionality
- File creation (empty and with content)
- File deletion and existence checking
- File information retrieval
- Binary file operations
- Path validation and security

### ✅ Storage Types
- **AppStorage:** Private app storage isolation
- **DocumentStorage:** User-accessible document storage
- Cross-storage operations and path verification
- Nested directory support

### ✅ Advanced Operations
- File copying within same storage
- File renaming and moving
- Directory creation and deletion
- Error condition handling

### ✅ Security & Edge Cases
- Path traversal prevention (`../`, absolute paths)
- Unicode filename support (Cyrillic, Chinese, Japanese, Korean, Greek, Emoji)
- Large file operations (1MB+ files)
- Special character handling
- Empty/null parameter validation

## Troubleshooting

### Common Issues

**Build Error: `'path' is unavailable: introduced in iOS 13.0`**
- **Solution:** Ensure deployment target is set to iOS 13.0 or later
- **Fix:** Update `CMAKE_OSX_DEPLOYMENT_TARGET=13.0`

**Simulator Launch Error: `domain=FBSOpenApplicationServiceErrorDomain, code=4`**
- **Solution:** Use `xcrun simctl spawn` instead of `xcrun simctl launch`
- **Alternative:** Run tests directly via executable path

**Code Signing Issues on Device**
- **Solution:** Set proper development team ID and code signing identity
- **Fix:** Update `CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM` with your Team ID

**Missing Foundation Framework**
- **Solution:** Ensure Objective-C++ compilation is enabled
- **Fix:** CMake automatically handles this with `enable_language(OBJCXX)`

### Debug Information

**Verbose Test Output:**
Tests include detailed logging showing:
- File paths being created/accessed
- Storage type verification
- Error conditions and security violations
- Unicode filename testing results

**Log Levels:**
- `INFO`: Normal operation details
- `ERROR`: Expected error conditions (security violations, invalid paths)

## Platform-Specific Notes

### ✅ macOS (TESTED)
- **Status:** ✅ All 27 tests passing
- **Implementation:** Uses standard POSIX file paths with Foundation framework
- **AppStorage:** `~/Library/Application Support/LeafraSDK/`
- **DocumentStorage:** `~/Documents/`
- **Last Tested:** Successfully verified on Apple Silicon

### ✅ iOS Simulator (TESTED)
- **Status:** ✅ All 27 tests passing on iPhone 16 Pro (iOS 18.3)
- **Implementation:** Sandboxed environment with simulator-specific paths
- **AppStorage:** `/Users/.../Library/Developer/CoreSimulator/Devices/<UDID>/data/Library/Application Support/com.leafra.filemanager.tests/`
- **DocumentStorage:** `/Users/.../Library/Developer/CoreSimulator/Devices/<UDID>/data/Documents/`
- **Framework Access:** Full Foundation framework access
- **Behavior:** Identical to device for file operations

### ⚠️ iOS Device (READY BUT UNTESTED)
- **Status:** ⚠️ Build system configured, awaiting device testing
- **Implementation:** Real device sandboxing and security
- **Requirements:** Apple Developer account, proper code signing and provisioning
- **Environment:** Production-equivalent testing environment

### ❌ Windows (NOT IMPLEMENTED)
- **Status:** ❌ Test build system not implemented
- **Implementation:** File manager code exists with Windows-specific paths
- **Required Work:** CMake configuration for Windows, Visual Studio project setup
- **Expected Paths:** 
  - AppStorage: `%LOCALAPPDATA%\LeafraSDK\`
  - DocumentStorage: `%USERPROFILE%\Documents\`

### ❌ Linux (NOT IMPLEMENTED)
- **Status:** ❌ Test build system not implemented  
- **Implementation:** File manager code exists with Linux-specific paths
- **Required Work:** CMake configuration for Linux, GCC/Clang setup
- **Expected Paths:**
  - AppStorage: `~/.local/share/LeafraSDK/`
  - DocumentStorage: `~/Documents/`

### ❌ Android (NOT IMPLEMENTED)
- **Status:** ❌ Test build system not implemented
- **Implementation:** File manager code exists with Android-specific paths
- **Required Work:** Android NDK integration, Gradle build system
- **Expected Paths:**
  - AppStorage: Internal app storage via Android APIs
  - DocumentStorage: External storage via Android APIs

## Contributing

When adding new tests:
1. Follow the existing test structure and naming conventions
2. Include both positive and negative test cases
3. Test across all supported platforms (currently: macOS ✅, iOS Simulator ✅)
4. Update this README with any new requirements or procedures

### Expanding Platform Support

**To add Windows/Linux/Android testing:**
1. **Windows:** Create CMake configuration for Visual Studio, add Windows-specific test runners
2. **Linux:** Add CMake configuration for GCC/Clang, create Linux build scripts  
3. **Android:** Integrate Android NDK, create Gradle build system for tests
4. **All Platforms:** Verify storage path implementations match expected behavior
5. **Update README:** Document new platform-specific build and run procedures

**Priority Order for Implementation:**
1. **iOS Device** (highest priority - build system ready, just needs testing)
2. **Windows** (high priority - large user base)
3. **Linux** (medium priority - development environment)
4. **Android** (medium priority - mobile platform completion)

## Support

For issues with the file manager tests:
1. Check the troubleshooting section above
2. Verify your development environment meets the prerequisites
3. Ensure proper code signing setup for iOS device testing
4. Review test logs for specific error details 