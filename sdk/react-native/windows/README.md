# React Native Windows Bridge for LeafraSDK

This directory contains the React Native Windows bridge implementation that connects JavaScript/TypeScript code to the LeafraSDK C++ library.

## Architecture

```
React Native (JS/TS) 
    ↓
Windows Bridge (C++/CLI or C++/WinRT)
    ↓
LeafraSDK Core (C++)
```

## Files to be implemented:

### C++/CLI Bridge (Option 1)
- `LeafraSDKModule.h` - React Native module header
- `LeafraSDKModule.cpp` - React Native module implementation
- `LeafraSDKBridge.h` - Bridge header
- `LeafraSDKBridge.cpp` - Bridge implementation

### C++/WinRT Bridge (Option 2)
- `LeafraSDKModule.idl` - WinRT interface definition
- `LeafraSDKModule.h` - Generated header
- `LeafraSDKModule.cpp` - Implementation
- `module.cpp` - Module registration

### Build Configuration
- `CMakeLists.txt` - Build configuration
- `LeafraSDK.vcxproj` - Visual Studio project file
- `packages.config` - NuGet packages

## Status: 🚧 Not Implemented

This bridge is planned but not yet implemented. The shared TypeScript interfaces in `../common/src/` will work once this native bridge is complete.

## Notes

React Native Windows supports both C++/CLI and C++/WinRT approaches. C++/WinRT is the modern recommended approach for new projects. 