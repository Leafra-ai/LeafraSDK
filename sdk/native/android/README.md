# Native Android Integration for LeafraSDK

This directory contains resources for integrating LeafraSDK directly into native Android applications using Kotlin or Java.

## Architecture

```
Android App (Kotlin/Java)
    ↓
Kotlin/Java Wrapper
    ↓
JNI Layer (C++)
    ↓
LeafraSDK Core (C++)
```

## Integration Options

### Option 1: Kotlin Wrapper (Recommended)
Create a Kotlin-friendly wrapper around the C++ SDK:

```kotlin
// LeafraSDK.kt
class LeafraSDK {
    companion object {
        init {
            System.loadLibrary("leafra-sdk")
        }
    }
    
    private external fun nativeInitialize(
        name: String?,
        version: String?,
        debugMode: Boolean,
        maxThreads: Int,
        bufferSize: Long
    ): Int
    
    private external fun nativeShutdown(): Int
    private external fun nativeIsInitialized(): Boolean
    private external fun nativeGetVersion(): String
    private external fun nativeGetPlatform(): String
    private external fun nativeProcessData(input: IntArray): ProcessDataResult
    private external fun nativeCalculateDistance2D(x1: Double, y1: Double, x2: Double, y2: Double): Double
    private external fun nativeCalculateDistance3D(x1: Double, y1: Double, z1: Double, x2: Double, y2: Double, z2: Double): Double
    
    fun initialize(config: LeafraConfig): ResultCode {
        val result = nativeInitialize(
            config.name,
            config.version,
            config.debugMode,
            config.maxThreads,
            config.bufferSize
        )
        return ResultCode.fromInt(result)
    }
    
    fun shutdown(): ResultCode {
        val result = nativeShutdown()
        return ResultCode.fromInt(result)
    }
    
    fun isInitialized(): Boolean = nativeIsInitialized()
    
    fun getVersion(): String = nativeGetVersion()
    
    fun getPlatform(): String = nativeGetPlatform()
    
    fun processData(input: IntArray): ProcessDataResult = nativeProcessData(input)
    
    fun calculateDistance2D(p1: Point2D, p2: Point2D): Double {
        return nativeCalculateDistance2D(p1.x, p1.y, p2.x, p2.y)
    }
    
    fun calculateDistance3D(p1: Point3D, p2: Point3D): Double {
        return nativeCalculateDistance3D(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z)
    }
}

// Supporting data classes
data class LeafraConfig(
    val name: String? = null,
    val version: String? = null,
    val debugMode: Boolean = false,
    val maxThreads: Int = 4,
    val bufferSize: Long = 1024
)

data class Point2D(val x: Double, val y: Double)
data class Point3D(val x: Double, val y: Double, val z: Double)

data class ProcessDataResult(
    val result: ResultCode,
    val output: IntArray
)

enum class ResultCode(val value: Int) {
    SUCCESS(0),
    ERROR_INITIALIZATION_FAILED(-1),
    ERROR_INVALID_PARAMETER(-2),
    ERROR_PROCESSING_FAILED(-3),
    ERROR_NOT_IMPLEMENTED(-4);
    
    companion object {
        fun fromInt(value: Int): ResultCode {
            return values().find { it.value == value } ?: ERROR_INITIALIZATION_FAILED
        }
    }
}
```

### Option 2: Java Wrapper
```java
// LeafraSDK.java
public class LeafraSDK {
    static {
        System.loadLibrary("leafra-sdk");
    }
    
    private native int nativeInitialize(String name, String version, boolean debugMode, int maxThreads, long bufferSize);
    private native int nativeShutdown();
    private native boolean nativeIsInitialized();
    private native String nativeGetVersion();
    private native String nativeGetPlatform();
    
    public ResultCode initialize(LeafraConfig config) {
        int result = nativeInitialize(
            config.getName(),
            config.getVersion(),
            config.isDebugMode(),
            config.getMaxThreads(),
            config.getBufferSize()
        );
        return ResultCode.fromInt(result);
    }
    
    public ResultCode shutdown() {
        return ResultCode.fromInt(nativeShutdown());
    }
    
    public boolean isInitialized() {
        return nativeIsInitialized();
    }
    
    public String getVersion() {
        return nativeGetVersion();
    }
    
    public String getPlatform() {
        return nativeGetPlatform();
    }
}
```

## JNI Implementation

```cpp
// leafra-sdk-jni.cpp
#include <jni.h>
#include "leafra/leafra_core.h"
#include <memory>

static std::shared_ptr<leafra::LeafraCore> g_sdk = nullptr;

extern "C" {

JNIEXPORT jint JNICALL
Java_com_leafra_sdk_LeafraSDK_nativeInitialize(
    JNIEnv *env, jobject thiz,
    jstring name, jstring version, jboolean debugMode,
    jint maxThreads, jlong bufferSize) {
    
    if (!g_sdk) {
        g_sdk = leafra::LeafraCore::create();
    }
    
    leafra::Config config;
    
    if (name) {
        const char* nameStr = env->GetStringUTFChars(name, nullptr);
        config.name = nameStr;
        env->ReleaseStringUTFChars(name, nameStr);
    }
    
    if (version) {
        const char* versionStr = env->GetStringUTFChars(version, nullptr);
        config.version = versionStr;
        env->ReleaseStringUTFChars(version, versionStr);
    }
    
    config.debug_mode = debugMode;
    config.max_threads = maxThreads;
    config.buffer_size = bufferSize;
    
    auto result = g_sdk->initialize(config);
    return static_cast<jint>(result);
}

JNIEXPORT jint JNICALL
Java_com_leafra_sdk_LeafraSDK_nativeShutdown(JNIEnv *env, jobject thiz) {
    if (g_sdk) {
        auto result = g_sdk->shutdown();
        g_sdk.reset();
        return static_cast<jint>(result);
    }
    return static_cast<jint>(leafra::ResultCode::ERROR_INITIALIZATION_FAILED);
}

JNIEXPORT jboolean JNICALL
Java_com_leafra_sdk_LeafraSDK_nativeIsInitialized(JNIEnv *env, jobject thiz) {
    return g_sdk ? g_sdk->is_initialized() : false;
}

JNIEXPORT jstring JNICALL
Java_com_leafra_sdk_LeafraSDK_nativeGetVersion(JNIEnv *env, jobject thiz) {
    std::string version = leafra::LeafraCore::get_version();
    return env->NewStringUTF(version.c_str());
}

JNIEXPORT jstring JNICALL
Java_com_leafra_sdk_LeafraSDK_nativeGetPlatform(JNIEnv *env, jobject thiz) {
    std::string platform = leafra::LeafraCore::get_platform();
    return env->NewStringUTF(platform.c_str());
}

JNIEXPORT jdouble JNICALL
Java_com_leafra_sdk_LeafraSDK_nativeCalculateDistance2D(
    JNIEnv *env, jobject thiz,
    jdouble x1, jdouble y1, jdouble x2, jdouble y2) {
    
    auto mathUtils = std::make_shared<leafra::MathUtils>();
    leafra::Point2D p1{x1, y1};
    leafra::Point2D p2{x2, y2};
    
    return mathUtils->calculate_distance_2d(p1, p2);
}

} // extern "C"
```

## Files to be created:

### Kotlin/Java Wrapper
- `src/main/java/com/leafra/sdk/LeafraSDK.kt` - Main Kotlin interface
- `src/main/java/com/leafra/sdk/LeafraConfig.kt` - Configuration data class
- `src/main/java/com/leafra/sdk/LeafraTypes.kt` - Data types and enums
- `src/main/java/com/leafra/sdk/LeafraException.kt` - Exception handling

### JNI Layer
- `src/main/cpp/leafra-sdk-jni.cpp` - JNI implementation
- `src/main/cpp/CMakeLists.txt` - CMake build configuration
- `src/main/cpp/Android.mk` - NDK build configuration

### Build Configuration
- `build.gradle` - Android module build script
- `proguard-rules.pro` - ProGuard rules for release builds
- `consumer-rules.pro` - Consumer ProGuard rules

## Integration Methods

### Gradle Dependency
```kotlin
// app/build.gradle
dependencies {
    implementation project(':leafra-sdk')
    // or
    implementation 'com.leafra:sdk:1.0.0'
}
```

### AAR Distribution
```kotlin
// Download and include AAR
implementation files('libs/leafra-sdk-1.0.0.aar')
```

## Usage Example

```kotlin
// MainActivity.kt
import com.leafra.sdk.LeafraSDK
import com.leafra.sdk.LeafraConfig
import com.leafra.sdk.Point2D
import com.leafra.sdk.ResultCode

class MainActivity : AppCompatActivity() {
    private val sdk = LeafraSDK()
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        
        // Initialize SDK
        val config = LeafraConfig(
            name = "MyAndroidApp",
            version = BuildConfig.VERSION_NAME,
            debugMode = BuildConfig.DEBUG,
            maxThreads = Runtime.getRuntime().availableProcessors(),
            bufferSize = 2048
        )
        
        when (sdk.initialize(config)) {
            ResultCode.SUCCESS -> {
                Log.d("LeafraSDK", "SDK initialized successfully")
                
                // Test distance calculation
                val p1 = Point2D(0.0, 0.0)
                val p2 = Point2D(3.0, 4.0)
                val distance = sdk.calculateDistance2D(p1, p2)
                Log.d("LeafraSDK", "Distance: $distance") // Should be 5.0
            }
            else -> {
                Log.e("LeafraSDK", "Failed to initialize SDK")
            }
        }
    }
    
    override fun onDestroy() {
        super.onDestroy()
        sdk.shutdown()
    }
}
```

## Build Configuration

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.18.1)
project("leafra-sdk")

# Add LeafraSDK core library
add_subdirectory(../../../../corecpp leafra-core)

# Create JNI library
add_library(leafra-sdk SHARED leafra-sdk-jni.cpp)

# Link libraries
target_link_libraries(leafra-sdk
    LeafraCore
    android
    log)

# Include directories
target_include_directories(leafra-sdk PRIVATE
    ../../../../corecpp/include)
```

### build.gradle
```kotlin
android {
    compileSdk 34
    
    defaultConfig {
        minSdk 23
        targetSdk 34
        
        ndk {
            abiFilters 'arm64-v8a', 'armeabi-v7a', 'x86', 'x86_64'
        }
    }
    
    externalNativeBuild {
        cmake {
            path "src/main/cpp/CMakeLists.txt"
            version "3.18.1"
        }
    }
}
```

## Status: 🚧 Not Implemented

This native Android integration is planned but not yet implemented. You can currently:

1. **Reuse React Native Bridge**: Use the existing bridge in `../react-native/android/` as a starting point
2. **Direct JNI Integration**: Create JNI bindings manually
3. **Wait for Kotlin Wrapper**: A dedicated Kotlin wrapper will be created

## Notes

- Android API 23+ (Android 6.0) required
- Supports ARM64, ARM32, x86, and x86_64 architectures
- Compatible with Kotlin and Java
- Thread-safe operations
- Automatic native library loading 