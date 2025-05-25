# Native macOS Integration for LeafraSDK

This directory contains resources for integrating LeafraSDK directly into native macOS applications using Swift or Objective-C.

## Architecture

```
macOS App (Swift/Objective-C)
    ↓
Swift Wrapper (Optional)
    ↓
Objective-C++ Bridge
    ↓
LeafraSDK Core (C++)
```

## Integration Options

### Option 1: Shared iOS Implementation
Since macOS and iOS both use the same Objective-C++ runtime, you can reuse the iOS native integration:

```swift
// Same Swift wrapper as iOS
import LeafraSDK

class AppDelegate: NSObject, NSApplicationDelegate {
    private let sdk = LeafraSDK()
    
    func applicationDidFinishLaunching(_ notification: Notification) {
        do {
            let config = LeafraConfig(
                name: "MyMacApp",
                version: "1.0.0",
                debugMode: true,
                maxThreads: 8, // macOS can handle more threads
                bufferSize: 2048
            )
            
            let result = try sdk.initialize(config: config)
            print("SDK initialized: \(result)")
        } catch {
            print("SDK initialization failed: \(error)")
        }
    }
}
```

### Option 2: Direct C++ Integration
Use the C++ SDK directly in your macOS app:

```objc
// In your .mm file
#include "leafra/leafra_core.h"

@implementation AppController

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    auto sdk = leafra::LeafraCore::create();
    leafra::Config config;
    config.name = "MyMacApp";
    config.debug_mode = true;
    config.max_threads = 8; // Take advantage of macOS multi-core
    
    auto result = sdk->initialize(config);
    if (result == leafra::ResultCode::SUCCESS) {
        NSLog(@"SDK initialized successfully on macOS");
    }
}

@end
```

## macOS-Specific Features

### Performance Optimizations
```swift
// Take advantage of macOS hardware
let config = LeafraConfig(
    name: "MyMacApp",
    version: "1.0.0",
    debugMode: false,
    maxThreads: ProcessInfo.processInfo.processorCount, // Use all cores
    bufferSize: 4096 // Larger buffer for desktop performance
)
```

### File System Integration
```swift
// macOS apps can process larger datasets
func processLargeDataFile(at url: URL) throws {
    let data = try Data(contentsOf: url)
    let inputArray = data.map { Int($0) }
    
    let result = try sdk.processData(input: inputArray)
    print("Processed \(result.output.count) bytes")
}
```

### Menu Bar Integration
```swift
// Add SDK controls to menu bar
@IBAction func initializeSDK(_ sender: NSMenuItem) {
    Task {
        do {
            let result = try sdk.initialize(config: defaultConfig)
            sender.title = result == .success ? "SDK Ready ✅" : "SDK Failed ❌"
        } catch {
            sender.title = "SDK Error ❌"
        }
    }
}
```

## Files to be created:

### Swift Wrapper (Recommended)
- `LeafraSDK.swift` - Main Swift interface (can reuse iOS version)
- `LeafraSDK+macOS.swift` - macOS-specific extensions
- `LeafraTypes.swift` - Swift data types (shared with iOS)
- `LeafraError.swift` - Error handling (shared with iOS)

### macOS-Specific Features
- `LeafraSDK+FileSystem.swift` - File processing utilities
- `LeafraSDK+Performance.swift` - Performance optimizations
- `LeafraSDK+MenuBar.swift` - Menu bar integration helpers

### Build Configuration
- `LeafraSDK.podspec` - CocoaPods (shared with iOS)
- `Package.swift` - Swift Package Manager
- `LeafraSDK.xcframework/` - Universal framework (iOS + macOS)

## Integration Methods

### CocoaPods
```ruby
# Podfile
platform :osx, '12.0'
pod 'LeafraSDK', :path => '../path/to/LeafraSDK/sdk'
```

### Swift Package Manager
```swift
// Package.swift
.target(
    name: "MyMacApp",
    dependencies: ["LeafraSDK"],
    platforms: [.macOS(.v12)]
)
```

### Manual Integration
1. Add `LeafraSDK.xcframework` to your macOS project
2. Ensure macOS 12.0+ deployment target
3. Link against `libc++`

## Usage Example

```swift
import Cocoa
import LeafraSDK

class ViewController: NSViewController {
    private let sdk = LeafraSDK()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        Task {
            do {
                // macOS-optimized configuration
                let config = LeafraConfig(
                    name: "MyMacApp",
                    version: Bundle.main.infoDictionary?["CFBundleShortVersionString"] as? String,
                    debugMode: false,
                    maxThreads: max(4, ProcessInfo.processInfo.processorCount),
                    bufferSize: 4096
                )
                
                let result = try sdk.initialize(config: config)
                print("SDK initialized: \(result)")
                
                // Process some data
                let testData = Array(1...1000)
                let processResult = try sdk.processData(input: testData)
                print("Processed \(processResult.output.count) elements")
                
            } catch {
                print("SDK error: \(error)")
            }
        }
    }
}
```

## Platform Differences from iOS

| Feature | iOS | macOS |
|---------|-----|-------|
| **Threading** | Limited cores | More CPU cores available |
| **Memory** | Constrained | More RAM available |
| **File System** | Sandboxed | More file access |
| **Performance** | Battery conscious | Performance focused |
| **UI Integration** | UIKit/SwiftUI | AppKit/SwiftUI |

## Status: ✅ Can Reuse iOS Implementation

The iOS native integration can be directly used on macOS with minimal changes:

1. **Shared Codebase**: Same Objective-C++ bridge works on both platforms
2. **Swift Compatibility**: Swift code is identical between platforms
3. **Framework Support**: Same `.xcframework` supports both iOS and macOS
4. **CocoaPods**: Same podspec works for both platforms

## Notes

- macOS 12.0+ required
- Supports both Intel and Apple Silicon Macs
- Compatible with AppKit and SwiftUI
- Can take advantage of more CPU cores and memory
- Better file system access than iOS 