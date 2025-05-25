# React Native macOS Bridge for LeafraSDK

This directory contains the React Native macOS bridge implementation that connects JavaScript/TypeScript code to the LeafraSDK C++ library.

## Architecture

```
React Native (JS/TS) 
    ↓
macOS Bridge (Objective-C++)
    ↓
LeafraSDK Core (C++)
```

## Implementation Options

### Option 1: Shared iOS Implementation (Recommended)
Since iOS and macOS both use Objective-C++ and have very similar APIs, the macOS bridge can reuse most of the iOS implementation:

```bash
# Create symbolic links to iOS implementation
ln -s ../ios/LeafraSDKModule.h LeafraSDKModule.h
ln -s ../ios/LeafraSDKModule.mm LeafraSDKModule.mm
ln -s ../ios/LeafraSDKBridge.h LeafraSDKBridge.h
ln -s ../ios/LeafraSDKBridge.mm LeafraSDKBridge.mm
```

### Option 2: Separate Implementation
If macOS-specific features are needed:

- `LeafraSDKModule.h` - React Native module header
- `LeafraSDKModule.mm` - React Native module implementation  
- `LeafraSDKBridge.h` - Bridge header
- `LeafraSDKBridge.mm` - Bridge implementation
- `CMakeLists.txt` - Build configuration

## Status: ✅ Can Use iOS Implementation

The iOS bridge implementation in `../ios/` already supports macOS through the same Objective-C++ codebase. No separate implementation is needed unless macOS-specific features are required.

## Notes

- React Native macOS is maintained by Microsoft
- The iOS bridge code is compatible with macOS with minimal changes
- Consider using symbolic links to avoid code duplication 