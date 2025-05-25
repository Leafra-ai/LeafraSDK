# React Native iOS Bridge for LeafraSDK

This directory contains the React Native iOS bridge implementation that connects JavaScript/TypeScript code to the LeafraSDK C++ library.

## Architecture

```
React Native (JS/TS) 
    ↓
iOS Bridge (Objective-C++)
    ↓
LeafraSDK Core (C++)
```

## Files

### React Native Module
- `LeafraSDKModule.h` - React Native module header extending RCTEventEmitter
- `LeafraSDKModule.mm` - React Native module implementation with RCT_EXPORT_METHOD macros

### Bridge Layer  
- `LeafraSDKBridge.h` - Objective-C++ bridge header with C++ integration methods
- `LeafraSDKBridge.mm` - Bridge implementation converting between Objective-C and C++ types

### Build Configuration
- `CMakeLists.txt` - CMake build configuration for iOS

## Status: ✅ Implemented

This bridge is fully implemented and working. It provides:

- ✅ SDK initialization and shutdown
- ✅ Data processing capabilities
- ✅ Mathematical operations (2D/3D distance, matrix operations)
- ✅ Event callback system
- ✅ Error handling with proper Result codes
- ✅ Memory management with proper weak references
- ✅ Thread-safe operations with dispatch queues

## Supported Platforms

- ✅ iOS 15.1+
- ✅ macOS 12.0+ (same codebase)
- ✅ iOS Simulator
- ✅ macOS (Apple Silicon and Intel)

## Integration

This bridge is integrated via CocoaPods using the `LeafraSDK.podspec` in the root SDK directory. The TypeScript interfaces are defined in `../common/src/index.ts`. 