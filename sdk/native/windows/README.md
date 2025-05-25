# Native Windows Integration for LeafraSDK

This directory contains resources for integrating LeafraSDK directly into native Windows applications using C#/.NET or C++.

## Architecture

```
Windows App (C#/.NET or C++)
    ↓
C#/.NET Wrapper (P/Invoke) or Direct C++
    ↓
LeafraSDK Core (C++)
```

## Integration Options

### Option 1: C#/.NET Wrapper (Recommended)
Create a C# wrapper using P/Invoke to call the C++ SDK:

```csharp
// LeafraSDK.cs
using System;
using System.Runtime.InteropServices;

namespace LeafraSDK
{
    public class LeafraSDK : IDisposable
    {
        private bool _disposed = false;
        
        // P/Invoke declarations
        [DllImport("leafra-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr leafra_create();
        
        [DllImport("leafra-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void leafra_destroy(IntPtr sdk);
        
        [DllImport("leafra-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int leafra_initialize(IntPtr sdk, ref LeafraConfig config);
        
        [DllImport("leafra-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int leafra_shutdown(IntPtr sdk);
        
        [DllImport("leafra-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool leafra_is_initialized(IntPtr sdk);
        
        [DllImport("leafra-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr leafra_get_version();
        
        [DllImport("leafra-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr leafra_get_platform();
        
        [DllImport("leafra-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern double leafra_calculate_distance_2d(double x1, double y1, double x2, double y2);
        
        [DllImport("leafra-sdk.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern double leafra_calculate_distance_3d(double x1, double y1, double z1, double x2, double y2, double z2);
        
        private IntPtr _sdkHandle;
        
        public LeafraSDK()
        {
            _sdkHandle = leafra_create();
            if (_sdkHandle == IntPtr.Zero)
            {
                throw new InvalidOperationException("Failed to create LeafraSDK instance");
            }
        }
        
        public ResultCode Initialize(LeafraConfig config)
        {
            if (_disposed) throw new ObjectDisposedException(nameof(LeafraSDK));
            
            var result = leafra_initialize(_sdkHandle, ref config);
            return (ResultCode)result;
        }
        
        public ResultCode Shutdown()
        {
            if (_disposed) throw new ObjectDisposedException(nameof(LeafraSDK));
            
            var result = leafra_shutdown(_sdkHandle);
            return (ResultCode)result;
        }
        
        public bool IsInitialized()
        {
            if (_disposed) throw new ObjectDisposedException(nameof(LeafraSDK));
            
            return leafra_is_initialized(_sdkHandle);
        }
        
        public string GetVersion()
        {
            var ptr = leafra_get_version();
            return Marshal.PtrToStringAnsi(ptr) ?? string.Empty;
        }
        
        public string GetPlatform()
        {
            var ptr = leafra_get_platform();
            return Marshal.PtrToStringAnsi(ptr) ?? string.Empty;
        }
        
        public double CalculateDistance2D(Point2D p1, Point2D p2)
        {
            if (_disposed) throw new ObjectDisposedException(nameof(LeafraSDK));
            
            return leafra_calculate_distance_2d(p1.X, p1.Y, p2.X, p2.Y);
        }
        
        public double CalculateDistance3D(Point3D p1, Point3D p2)
        {
            if (_disposed) throw new ObjectDisposedException(nameof(LeafraSDK));
            
            return leafra_calculate_distance_3d(p1.X, p1.Y, p1.Z, p2.X, p2.Y, p2.Z);
        }
        
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }
        
        protected virtual void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (_sdkHandle != IntPtr.Zero)
                {
                    leafra_destroy(_sdkHandle);
                    _sdkHandle = IntPtr.Zero;
                }
                _disposed = true;
            }
        }
        
        ~LeafraSDK()
        {
            Dispose(false);
        }
    }
    
    // Supporting types
    [StructLayout(LayoutKind.Sequential)]
    public struct LeafraConfig
    {
        [MarshalAs(UnmanagedType.LPStr)]
        public string Name;
        
        [MarshalAs(UnmanagedType.LPStr)]
        public string Version;
        
        [MarshalAs(UnmanagedType.Bool)]
        public bool DebugMode;
        
        public int MaxThreads;
        public ulong BufferSize;
    }
    
    public struct Point2D
    {
        public double X { get; set; }
        public double Y { get; set; }
        
        public Point2D(double x, double y)
        {
            X = x;
            Y = y;
        }
    }
    
    public struct Point3D
    {
        public double X { get; set; }
        public double Y { get; set; }
        public double Z { get; set; }
        
        public Point3D(double x, double y, double z)
        {
            X = x;
            Y = y;
            Z = z;
        }
    }
    
    public enum ResultCode
    {
        Success = 0,
        ErrorInitializationFailed = -1,
        ErrorInvalidParameter = -2,
        ErrorProcessingFailed = -3,
        ErrorNotImplemented = -4
    }
}
```

### Option 2: Direct C++ Integration
Use the C++ SDK directly in a C++ Windows application:

```cpp
// main.cpp
#include "leafra/leafra_core.h"
#include <iostream>
#include <memory>

int main()
{
    // Create SDK instance
    auto sdk = leafra::LeafraCore::create();
    
    // Configure SDK
    leafra::Config config;
    config.name = "MyWindowsApp";
    config.version = "1.0.0";
    config.debug_mode = true;
    config.max_threads = std::thread::hardware_concurrency();
    config.buffer_size = 2048;
    
    // Initialize
    auto result = sdk->initialize(config);
    if (result == leafra::ResultCode::SUCCESS) {
        std::cout << "SDK initialized successfully" << std::endl;
        
        // Test distance calculation
        leafra::Point2D p1{0.0, 0.0};
        leafra::Point2D p2{3.0, 4.0};
        
        auto mathUtils = std::make_shared<leafra::MathUtils>();
        double distance = mathUtils->calculate_distance_2d(p1, p2);
        std::cout << "Distance: " << distance << std::endl; // Should be 5.0
        
        // Shutdown
        sdk->shutdown();
    } else {
        std::cerr << "Failed to initialize SDK" << std::endl;
        return -1;
    }
    
    return 0;
}
```

### Option 3: WinRT/UWP Integration
For Universal Windows Platform apps:

```cpp
// LeafraSDKWinRT.h
#pragma once
#include "winrt/base.h"
#include "winrt/Windows.Foundation.h"

namespace winrt::LeafraSDK::implementation
{
    struct LeafraSDKWinRT : LeafraSDKWinRTT<LeafraSDKWinRT>
    {
        LeafraSDKWinRT();
        
        Windows::Foundation::IAsyncOperation<int32_t> InitializeAsync(hstring name, hstring version, bool debugMode);
        int32_t Shutdown();
        bool IsInitialized();
        hstring GetVersion();
        hstring GetPlatform();
        double CalculateDistance2D(double x1, double y1, double x2, double y2);
        
    private:
        std::shared_ptr<leafra::LeafraCore> m_sdk;
    };
}
```

## C API Wrapper

To support P/Invoke, create a C API wrapper:

```cpp
// leafra_c_api.cpp
#include "leafra/leafra_core.h"
#include <memory>
#include <unordered_map>

extern "C" {

static std::unordered_map<void*, std::shared_ptr<leafra::LeafraCore>> g_sdk_instances;

__declspec(dllexport) void* leafra_create()
{
    auto sdk = leafra::LeafraCore::create();
    void* handle = sdk.get();
    g_sdk_instances[handle] = sdk;
    return handle;
}

__declspec(dllexport) void leafra_destroy(void* sdk)
{
    if (sdk && g_sdk_instances.find(sdk) != g_sdk_instances.end()) {
        g_sdk_instances.erase(sdk);
    }
}

__declspec(dllexport) int leafra_initialize(void* sdk, const leafra_config_t* config)
{
    if (!sdk || g_sdk_instances.find(sdk) == g_sdk_instances.end()) {
        return static_cast<int>(leafra::ResultCode::ERROR_INVALID_PARAMETER);
    }
    
    leafra::Config cppConfig;
    if (config->name) cppConfig.name = config->name;
    if (config->version) cppConfig.version = config->version;
    cppConfig.debug_mode = config->debug_mode;
    cppConfig.max_threads = config->max_threads;
    cppConfig.buffer_size = config->buffer_size;
    
    auto result = g_sdk_instances[sdk]->initialize(cppConfig);
    return static_cast<int>(result);
}

__declspec(dllexport) int leafra_shutdown(void* sdk)
{
    if (!sdk || g_sdk_instances.find(sdk) == g_sdk_instances.end()) {
        return static_cast<int>(leafra::ResultCode::ERROR_INVALID_PARAMETER);
    }
    
    auto result = g_sdk_instances[sdk]->shutdown();
    return static_cast<int>(result);
}

__declspec(dllexport) bool leafra_is_initialized(void* sdk)
{
    if (!sdk || g_sdk_instances.find(sdk) == g_sdk_instances.end()) {
        return false;
    }
    
    return g_sdk_instances[sdk]->is_initialized();
}

__declspec(dllexport) const char* leafra_get_version()
{
    static std::string version = leafra::LeafraCore::get_version();
    return version.c_str();
}

__declspec(dllexport) const char* leafra_get_platform()
{
    static std::string platform = leafra::LeafraCore::get_platform();
    return platform.c_str();
}

__declspec(dllexport) double leafra_calculate_distance_2d(double x1, double y1, double x2, double y2)
{
    auto mathUtils = std::make_shared<leafra::MathUtils>();
    leafra::Point2D p1{x1, y1};
    leafra::Point2D p2{x2, y2};
    return mathUtils->calculate_distance_2d(p1, p2);
}

} // extern "C"
```

## Files to be created:

### C#/.NET Wrapper
- `LeafraSDK.cs` - Main C# interface
- `LeafraTypes.cs` - Data types and enums
- `LeafraException.cs` - Exception handling
- `LeafraSDK.csproj` - .NET project file

### C API Layer
- `leafra_c_api.h` - C API header
- `leafra_c_api.cpp` - C API implementation
- `leafra-sdk.def` - DLL export definitions

### Build Configuration
- `CMakeLists.txt` - CMake build configuration
- `LeafraSDK.vcxproj` - Visual Studio project
- `packages.config` - NuGet packages

## Integration Methods

### NuGet Package
```xml
<!-- .csproj -->
<PackageReference Include="LeafraSDK" Version="1.0.0" />
```

### Manual DLL
```csharp
// Copy leafra-sdk.dll to output directory
// Reference in project or use DllImport with full path
```

## Usage Example

```csharp
// Program.cs
using System;
using LeafraSDK;

class Program
{
    static void Main(string[] args)
    {
        using var sdk = new LeafraSDK.LeafraSDK();
        
        try
        {
            var config = new LeafraConfig
            {
                Name = "MyWindowsApp",
                Version = "1.0.0",
                DebugMode = true,
                MaxThreads = Environment.ProcessorCount,
                BufferSize = 2048
            };
            
            var result = sdk.Initialize(config);
            if (result == ResultCode.Success)
            {
                Console.WriteLine("SDK initialized successfully");
                Console.WriteLine($"Version: {sdk.GetVersion()}");
                Console.WriteLine($"Platform: {sdk.GetPlatform()}");
                
                // Test distance calculation
                var p1 = new Point2D(0, 0);
                var p2 = new Point2D(3, 4);
                var distance = sdk.CalculateDistance2D(p1, p2);
                Console.WriteLine($"Distance: {distance}"); // Should be 5.0
                
                sdk.Shutdown();
            }
            else
            {
                Console.WriteLine($"Failed to initialize SDK: {result}");
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error: {ex.Message}");
        }
    }
}
```

## Build Configuration

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.20)
project(LeafraSDK-Windows)

# Add LeafraSDK core library
add_subdirectory(../../../corecpp leafra-core)

# Create C API DLL
add_library(leafra-sdk SHARED 
    leafra_c_api.cpp
    leafra_c_api.h
    leafra-sdk.def)

target_link_libraries(leafra-sdk LeafraCore)

# Set DLL properties
set_target_properties(leafra-sdk PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
```

### Visual Studio Project
```xml
<!-- LeafraSDK.vcxproj -->
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup>
    <ConfigurationType>DynamicLibrary</ConfigurationType>
    <PlatformToolset>v143</PlatformToolset>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  
  <ItemGroup>
    <ClCompile Include="leafra_c_api.cpp" />
  </ItemGroup>
  
  <ItemGroup>
    <ClInclude Include="leafra_c_api.h" />
  </ItemGroup>
</Project>
```

## Status: 🚧 Not Implemented

This native Windows integration is planned but not yet implemented. You can currently:

1. **Reuse React Native Bridge**: Use the existing bridge in `../react-native/windows/` as a starting point
2. **Direct C++ Integration**: Include C++ headers directly in Windows C++ projects
3. **Wait for C# Wrapper**: A dedicated C#/.NET wrapper will be created

## Notes

- Windows 10+ required
- Supports x64 and ARM64 architectures
- Compatible with .NET 6+, .NET Framework 4.8+
- Visual Studio 2022 recommended
- Thread-safe operations
- Automatic DLL loading 