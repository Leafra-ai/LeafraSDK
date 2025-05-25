# Native iOS Integration for LeafraSDK

This directory contains resources for integrating LeafraSDK directly into native iOS applications using Swift or Objective-C.

## Architecture

```
iOS App (Swift/Objective-C)
    ↓
Swift Wrapper (Optional)
    ↓
Objective-C++ Bridge
    ↓
LeafraSDK Core (C++)
```

## Integration Options

### Option 1: Direct C++ Integration
Use the C++ SDK directly in your iOS app:

```objc
// In your .mm file
#include "leafra/leafra_core.h"

@implementation MyViewController

- (void)initializeSDK {
    auto sdk = leafra::LeafraCore::create();
    leafra::Config config;
    config.name = "MyApp";
    config.debug_mode = true;
    
    auto result = sdk->initialize(config);
    if (result == leafra::ResultCode::SUCCESS) {
        NSLog(@"SDK initialized successfully");
    }
}

@end
```

### Option 2: Swift Wrapper (Recommended)
Create a Swift-friendly wrapper around the C++ SDK:

```swift
// LeafraSDK.swift
import Foundation

public class LeafraSDK {
    private let bridge: LeafraSDKBridge
    
    public init() {
        bridge = LeafraSDKBridge()
    }
    
    public func initialize(config: LeafraConfig) throws -> ResultCode {
        var error: NSError?
        let result = bridge.initialize(with: config.toDictionary(), error: &error)
        
        if let error = error {
            throw error
        }
        
        return ResultCode(rawValue: result.intValue) ?? .errorInitializationFailed
    }
    
    public func calculateDistance2D(from p1: Point2D, to p2: Point2D) throws -> Double {
        var error: NSError?
        let distance = bridge.calculateDistance2D(p1.toDictionary(), 
                                                 point2: p2.toDictionary(), 
                                                 error: &error)
        
        if let error = error {
            throw error
        }
        
        return distance.doubleValue
    }
}

// Supporting types
public struct LeafraConfig {
    public let name: String?
    public let version: String?
    public let debugMode: Bool
    public let maxThreads: Int
    public let bufferSize: UInt
    
    func toDictionary() -> [String: Any] {
        var dict: [String: Any] = [:]
        if let name = name { dict["name"] = name }
        if let version = version { dict["version"] = version }
        dict["debugMode"] = debugMode
        dict["maxThreads"] = maxThreads
        dict["bufferSize"] = bufferSize
        return dict
    }
}

public struct Point2D {
    public let x: Double
    public let y: Double
    
    func toDictionary() -> [String: Double] {
        return ["x": x, "y": y]
    }
}

public enum ResultCode: Int {
    case success = 0
    case errorInitializationFailed = -1
    case errorInvalidParameter = -2
    case errorProcessingFailed = -3
    case errorNotImplemented = -4
}
```

## Files to be created:

### Swift Wrapper (Recommended)
- `LeafraSDK.swift` - Main Swift interface
- `LeafraTypes.swift` - Swift data types and enums
- `LeafraError.swift` - Swift error handling
- `LeafraSDK+Extensions.swift` - Convenience extensions

### Objective-C++ Bridge (if not using React Native bridge)
- `LeafraSDKBridge.h` - Bridge header
- `LeafraSDKBridge.mm` - Bridge implementation
- `LeafraSDK-Bridging-Header.h` - Swift bridging header

### Build Configuration
- `LeafraSDK.podspec` - CocoaPods specification
- `Package.swift` - Swift Package Manager support
- `LeafraSDK.xcframework/` - Pre-built framework

## Integration Methods

### CocoaPods
```ruby
# Podfile
pod 'LeafraSDK', :path => '../path/to/LeafraSDK/sdk'
```

### Swift Package Manager
```swift
// Package.swift
dependencies: [
    .package(url: "https://github.com/your-org/LeafraSDK.git", from: "1.0.0")
]
```

### Manual Integration
1. Add `LeafraSDK.xcframework` to your project
2. Link against `libc++`
3. Add bridging header for Swift projects

## Usage Example

```swift
import LeafraSDK

class ViewController: UIViewController {
    private let sdk = LeafraSDK()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        do {
            let config = LeafraConfig(
                name: "MyApp",
                version: "1.0.0",
                debugMode: true,
                maxThreads: 4,
                bufferSize: 1024
            )
            
            let result = try sdk.initialize(config: config)
            print("SDK initialized with result: \(result)")
            
            // Calculate distance
            let p1 = Point2D(x: 0, y: 0)
            let p2 = Point2D(x: 3, y: 4)
            let distance = try sdk.calculateDistance2D(from: p1, to: p2)
            print("Distance: \(distance)") // Should be 5.0
            
        } catch {
            print("SDK error: \(error)")
        }
    }
}
```

## Status: 🚧 Not Implemented

This native integration is planned but not yet implemented. You can currently:

1. **Reuse React Native Bridge**: Use the existing bridge in `../react-native/ios/` as a starting point
2. **Direct C++ Integration**: Include C++ headers directly in `.mm` files
3. **Wait for Swift Wrapper**: A dedicated Swift wrapper will be created

## Notes

- iOS 15.1+ required
- Supports both iPhone and iPad
- Compatible with SwiftUI and UIKit
- Thread-safe operations
- Automatic memory management 