# LeafraSDK SQLite Unit Tests

This directory contains comprehensive unit tests for the LeafraSDK SQLite integration, testing database operations, security features, and cross-platform compatibility.

## Test Suites

The tests are organized into two comprehensive suites:

1. **`test_sqlite_basic`** - Core SQLite operations
   - Database creation and opening with path validation
   - Basic SQL operations (CREATE, INSERT, SELECT, UPDATE, DELETE)
   - RAG tables schema verification
   - In-memory database operations
   - Security validation (path traversal prevention)

2. **`test_sqlite_advanced`** - Advanced SQLite features
   - Prepared statements with parameter binding
   - Transaction management (commit, rollback, RAII)
   - Error handling and reporting
   - Data type handling (INTEGER, REAL, TEXT, BLOB, NULL)
   - Column access methods (by index and name)

**Total Coverage:** 117 tests across all critical SQLite functionality

## Platform Testing Status

### ✅ FULLY TESTED & VERIFIED (100% Pass Rate)
- **macOS (Apple Silicon)** - 117/117 tests passing
  - Basic Operations: 46/46 tests ✅
  - Advanced Features: 71/71 tests ✅
  - System SQLite integration via macOS SDK
  - Native performance and security
- **iOS Simulator (iPhone 16 Pro, iOS 18.2)** - 117/117 tests passing
  - Basic Operations: 46/46 tests ✅
  - Advanced Features: 71/71 tests ✅
  - ARM64 architecture on Apple Silicon Mac
  - Full iOS sandbox compatibility

### ⚠️ READY BUT UNTESTED
- **iOS Device** - Build system configured, should work identically to simulator
  - Requires Apple Developer account for code signing
  - Same SQLite integration as simulator
  - Expected: 117/117 tests should pass

### ❌ NOT YET IMPLEMENTED
- **Windows** - SQLite integration exists but unit test build system not configured
- **Linux** - SQLite integration exists but unit test build system not configured  
- **Android** - SQLite integration exists but unit test build system not configured

## Prerequisites

### macOS
- **Xcode 14+** with Command Line Tools
- **CMake 3.20+**
- **System SQLite** (included with macOS)

### iOS Simulator/Device
- **Xcode 14+** with iOS SDK
- **CMake 3.20+** with iOS toolchain support
- **iOS 13.0+** deployment target
- **Apple Developer Account** (for device testing)

### Windows (TODO)
- **Visual Studio 2019+** or **MinGW**
- **CMake 3.20+**
- **SQLite library** (to be configured)

### Linux (TODO)
- **GCC 9+** or **Clang 10+**
- **CMake 3.20+**
- **SQLite development package** (`libsqlite3-dev`)

### Android (TODO)
- **Android NDK**
- **CMake 3.20+**
- **SQLite for Android**

## Building and Running Tests

### macOS Native

```bash
# Navigate to SQLite tests directory
cd sdk/corecpp/src/unit_tests/sqlite

# Build tests
mkdir build_macos && cd build_macos
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Run tests
./test_sqlite_basic
./test_sqlite_advanced

# Or use CTest
ctest --verbose
```

**Expected Output:**
```
🧪 LeafraSDK SQLite Unit Tests - Basic Operations
=================================================
✅ PASS: Create new database with valid relative path
✅ PASS: Database file exists after creation
...
📊 Test Results: 46/46 tests passed
🎉 All tests passed!

🧪 LeafraSDK SQLite Unit Tests - Advanced Features
==================================================
✅ PASS: Prepared statement with positional parameters
✅ PASS: Transaction commit and rollback
...
📊 Test Results: 71/71 tests passed
🎉 All tests passed!
```

### iOS Simulator

#### Option 1: Using Existing Simulator
```bash
# Ensure iOS Simulator is running
open -a Simulator

# Build for iOS Simulator (ARM64 for Apple Silicon Mac)
mkdir build_ios && cd build_ios
cmake .. -DCMAKE_SYSTEM_NAME=iOS \
         -DCMAKE_OSX_SYSROOT=iphonesimulator \
         -DCMAKE_OSX_ARCHITECTURES=arm64 \
         -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0

# Build tests
make -j$(nproc)

# Run tests on simulator (direct execution)
xcrun simctl spawn booted ./test_sqlite_basic
xcrun simctl spawn booted ./test_sqlite_advanced
```

#### Option 2: Launch New Simulator
```bash
# Build tests (same as above)
# ...

# Launch specific simulator and run tests
xcrun simctl boot "iPhone 16 Pro"
xcrun simctl spawn "iPhone 16 Pro" ./test_sqlite_basic
xcrun simctl spawn "iPhone 16 Pro" ./test_sqlite_advanced
```

### iOS Device

```bash
# Set your development team ID
export IOS_DEVELOPMENT_TEAM="YOUR_TEAM_ID"

# Build for iOS Device
mkdir build_ios_device && cd build_ios_device
cmake .. -DCMAKE_SYSTEM_NAME=iOS \
         -DCMAKE_OSX_ARCHITECTURES=arm64 \
         -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
         -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM="$IOS_DEVELOPMENT_TEAM" \
         -G Xcode

# Build and deploy
xcodebuild -project LeafraSQLiteTests.xcodeproj \
           -scheme test_sqlite_basic \
           -configuration Debug \
           -destination 'generic/platform=iOS' \
           build

# Install and run on connected device
# (Requires device to be connected and trusted)
```

### Windows (Placeholder)

```cmd
REM TODO: Windows build instructions
REM mkdir build_windows
REM cmake .. -G "Visual Studio 16 2019" -A x64
REM cmake --build build_windows --config Debug
REM build_windows\Debug\test_sqlite_basic.exe
```

### Linux (Placeholder)

```bash
# TODO: Linux build instructions
# sudo apt-get install libsqlite3-dev  # Ubuntu/Debian
# mkdir build_linux && cd build_linux
# cmake .. -DCMAKE_BUILD_TYPE=Debug
# make -j$(nproc)
# ./test_sqlite_basic
```

### Android (Placeholder)

```bash
# TODO: Android build instructions
# mkdir build_android && cd build_android
# cmake .. -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
#          -DANDROID_ABI=arm64-v8a \
#          -DANDROID_PLATFORM=android-21
# make -j$(nproc)
```

## Test Features

### Security Testing
- **Path Validation:** Rejects absolute paths and path traversal attempts
- **File Manager Integration:** Uses secure AppStorage location
- **Input Sanitization:** Tests SQL injection prevention

### Database Operations
- **CRUD Operations:** Complete Create, Read, Update, Delete testing
- **Schema Management:** RAG tables creation and validation
- **Data Types:** All SQLite data types (INTEGER, REAL, TEXT, BLOB, NULL)

### Advanced Features
- **Prepared Statements:** Parameter binding and reuse
- **Transactions:** ACID compliance and rollback testing
- **Error Handling:** Comprehensive error condition coverage
- **Memory Databases:** In-memory operation testing

### Cross-Platform Compatibility
- **Path Handling:** Platform-specific path resolution
- **SQLite Integration:** System vs. bundled SQLite libraries
- **Framework Dependencies:** Apple frameworks on iOS/macOS

## Storage Locations

### macOS
- **AppStorage:** `~/Library/Application Support/com.leafra.sqlite.<test_name>/`
- **Test Databases:** Created in secure AppStorage location

### iOS Simulator
- **AppStorage:** `/Users/.../Library/Developer/CoreSimulator/Devices/.../data/Containers/Data/Application/.../Library/Application Support/`
- **Sandboxed:** Each test app has isolated storage

### iOS Device
- **AppStorage:** Sandboxed application-specific directory
- **Security:** Full iOS security model enforcement

## Troubleshooting

### Common Issues

#### "Database file not found"
- Ensure file manager is properly initialized
- Check AppStorage permissions
- Verify relative path format

#### "SQLite library not found" (Linux)
```bash
# Ubuntu/Debian
sudo apt-get install libsqlite3-dev

# CentOS/RHEL
sudo yum install sqlite-devel
```

#### iOS Simulator "App not found"
- Ensure simulator is booted
- Check app bundle was created correctly
- Verify deployment target compatibility

#### Code Signing Issues (iOS Device)
- Set valid development team ID
- Ensure device is registered
- Check provisioning profile

### Debug Output

Enable verbose logging by setting environment variable:
```bash
export LEAFRA_LOG_LEVEL=DEBUG
./test_sqlite_basic
```

## Contributing

When adding new tests:
1. Follow existing test structure and naming conventions
2. Include both positive and negative test cases
3. Test across all supported platforms (currently: macOS ✅, iOS Simulator ✅)
4. Update this README with any new requirements or procedures

### Expanding Platform Support

**To add Windows/Linux/Android testing:**
1. **Windows:** Complete CMake configuration for Visual Studio, configure SQLite linking (system or bundled)
2. **Linux:** Complete CMake configuration for GCC/Clang, add system SQLite package integration
3. **Android:** Integrate Android NDK, create Gradle build scripts, configure Android SQLite setup

**Current Implementation Status:**
- Core SQLite integration (`leafra_sqlite.cpp`) is cross-platform ready
- CMake build system has Apple platform support complete
- Test suites are platform-agnostic and ready for expansion
- Only platform-specific build configuration needed for remaining platforms

## Integration with LeafraSDK

These tests validate the SQLite integration used throughout LeafraSDK:
- **Document Storage:** RAG tables for document indexing
- **Embedding Storage:** BLOB storage for vector embeddings
- **Metadata Management:** Structured data storage
- **Cross-Platform Compatibility:** Consistent behavior across platforms

The tests ensure that SQLite operations work reliably across all target platforms with proper security and performance characteristics. 