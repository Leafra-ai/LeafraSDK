# React Native Android Bridge for LeafraSDK

This directory contains the React Native Android bridge implementation that connects JavaScript/TypeScript code to the LeafraSDK C++ library.

## Architecture

```
React Native (JS/TS) 
    ↓
Android Bridge (Java/Kotlin)
    ↓  
JNI Layer (C++)
    ↓
LeafraSDK Core (C++)
```

## Files to be implemented:

### Java/Kotlin Bridge
- `src/main/java/com/leafra/sdk/LeafraSDKModule.java` - Main React Native module
- `src/main/java/com/leafra/sdk/LeafraSDKPackage.java` - Package registration
- `src/main/java/com/leafra/sdk/LeafraSDKBridge.java` - Bridge to JNI layer

### JNI Layer
- `src/main/cpp/leafra-sdk-jni.cpp` - JNI implementation
- `src/main/cpp/CMakeLists.txt` - Build configuration
- `src/main/cpp/Android.mk` - Alternative build configuration

### Build Configuration
- `build.gradle` - Android build configuration
- `proguard-rules.pro` - ProGuard rules

## Status: 🚧 Not Implemented

This bridge is planned but not yet implemented. The shared TypeScript interfaces in `../common/src/` will work once this native bridge is complete. 