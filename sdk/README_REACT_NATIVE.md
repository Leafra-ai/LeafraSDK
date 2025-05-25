# LeafraSDK React Native Integration

This document explains how to integrate and use the LeafraSDK React Native bridge for iOS and macOS platforms.

## 📁 File Structure

The React Native bridge consists of the following key files:

```
sdk/
├── ios/
│   ├── LeafraSDKModule.h          # React Native module header
│   ├── LeafraSDKModule.mm         # React Native module implementation
│   ├── LeafraSDKBridge.h          # Objective-C++ bridge header
│   └── LeafraSDKBridge.mm         # Objective-C++ bridge implementation
├── react-native/
│   ├── src/index.ts               # TypeScript interface
│   ├── package.json               # NPM package configuration
│   └── tsconfig.json              # TypeScript configuration
├── corecpp/
│   ├── include/leafra/            # C++ headers
│   └── src/                       # C++ implementations
└── LeafraSDK.podspec              # CocoaPods specification
```

## 🚀 Integration Steps

### 1. Add to Your React Native Project

#### Option A: Using CocoaPods (Recommended)

Add to your `ios/Podfile`:

```ruby
pod 'LeafraSDK', :path => '../path/to/LeafraSDK/sdk'
```

Then run:
```bash
cd ios && pod install
```

#### Option B: Manual Integration

1. Add the SDK to your `react-native.config.js`:

```javascript
const path = require('path');

module.exports = {
  dependencies: {
    'react-native-leafra-sdk': {
      root: path.join(__dirname, '../path/to/LeafraSDK/sdk/react-native'),
      platforms: {
        ios: {
          sourceDir: path.join(__dirname, '../path/to/LeafraSDK/sdk/react-native/ios'),
          podspecPath: path.join(__dirname, '../path/to/LeafraSDK/sdk/LeafraSDK.podspec'),
        },
      },
    },
  },
};
```

### 2. Install Dependencies

```bash
npm install ../path/to/LeafraSDK/sdk/react-native
# or
yarn add ../path/to/LeafraSDK/sdk/react-native
```

### 3. iOS Project Configuration

Ensure your iOS project has the following settings:

- **Deployment Target**: iOS 15.1+
- **C++ Language Standard**: C++17
- **C++ Standard Library**: libc++

## 💻 Usage Examples

### Basic Initialization

```typescript
import React, { useEffect, useState } from 'react';
import { LeafraSDK, LeafraConfig, ResultCode } from 'react-native-leafra-sdk';

const App = () => {
  const [sdkInitialized, setSdkInitialized] = useState(false);

  useEffect(() => {
    initializeSDK();
  }, []);

  const initializeSDK = async () => {
    try {
      const config: LeafraConfig = {
        name: 'MyApp',
        version: '1.0.0',
        debugMode: __DEV__,
        maxThreads: 4,
        bufferSize: 1024
      };

      const sdk = LeafraSDK.getInstance();
      const result = await sdk.initialize(config);

      if (result === ResultCode.SUCCESS) {
        console.log('✅ LeafraSDK initialized successfully');
        setSdkInitialized(true);
      } else {
        console.error('❌ Failed to initialize LeafraSDK:', result);
      }
    } catch (error) {
      console.error('💥 SDK initialization error:', error);
    }
  };

  return (
    // Your app components
  );
};
```

### Data Processing

```typescript
const processData = async () => {
  const sdk = LeafraSDK.getInstance();
  
  try {
    const inputData = [1, 2, 3, 4, 5];
    const result = await sdk.processData(inputData);
    
    if (result.result === ResultCode.SUCCESS) {
      console.log('📊 Processed data:', result.output);
    } else {
      console.error('❌ Data processing failed:', result.result);
    }
  } catch (error) {
    console.error('💥 Data processing error:', error);
  }
};
```

### Mathematical Operations

```typescript
const performMathOperations = async () => {
  const sdk = LeafraSDK.getInstance();
  
  try {
    // Calculate 2D distance
    const p1 = { x: 0, y: 0 };
    const p2 = { x: 3, y: 4 };
    const distance2D = await sdk.calculateDistance2D(p1, p2);
    console.log('📐 2D Distance:', distance2D); // Should be 5.0
    
    // Calculate 3D distance
    const p3d1 = { x: 0, y: 0, z: 0 };
    const p3d2 = { x: 1, y: 1, z: 1 };
    const distance3D = await sdk.calculateDistance3D(p3d1, p3d2);
    console.log('📐 3D Distance:', distance3D); // Should be √3 ≈ 1.732
    
    // Matrix operations
    const matrixA = { data: [1, 0, 0, 0, 1, 0, 0, 0, 1] }; // Identity matrix
    const matrixB = { data: [2, 0, 0, 0, 2, 0, 0, 0, 2] }; // 2x identity
    
    const multiplyResult = await sdk.multiplyMatrix3x3(matrixA, matrixB);
    if (multiplyResult.result === ResultCode.SUCCESS) {
      console.log('🔢 Matrix multiplication result:', multiplyResult.matrix);
    }
    
    const determinant = await sdk.matrixDeterminant(matrixA);
    console.log('🔢 Matrix determinant:', determinant); // Should be 1.0
    
  } catch (error) {
    console.error('💥 Math operations error:', error);
  }
};
```

### Event Handling

```typescript
const setupEventHandling = () => {
  const sdk = LeafraSDK.getInstance();
  
  // Set up event callback
  sdk.setEventCallback((message: string) => {
    console.log('📢 SDK Event:', message);
    // Handle SDK events (initialization, processing, errors, etc.)
  });
};
```

### Complete Example

```typescript
import React, { useEffect, useState } from 'react';
import { View, Text, Button, StyleSheet } from 'react-native';
import { LeafraSDK, LeafraConfig, ResultCode } from 'react-native-leafra-sdk';

const LeafraExample = () => {
  const [sdkStatus, setSdkStatus] = useState('Not initialized');
  const [results, setResults] = useState<string[]>([]);

  useEffect(() => {
    initializeSDK();
    return () => {
      // Cleanup on unmount
      LeafraSDK.getInstance().shutdown();
    };
  }, []);

  const initializeSDK = async () => {
    try {
      const config: LeafraConfig = {
        name: 'LeafraExample',
        version: '1.0.0',
        debugMode: true,
        maxThreads: 2,
        bufferSize: 512
      };

      const sdk = LeafraSDK.getInstance();
      
      // Set up event handling
      sdk.setEventCallback((message) => {
        addResult(`Event: ${message}`);
      });

      const result = await sdk.initialize(config);
      
      if (result === ResultCode.SUCCESS) {
        setSdkStatus('Initialized');
        addResult('✅ SDK initialized successfully');
        
        // Get SDK info
        const version = await sdk.getVersion();
        const platform = await sdk.getPlatform();
        addResult(`📱 Platform: ${platform}, Version: ${version}`);
      } else {
        setSdkStatus('Failed');
        addResult(`❌ Initialization failed: ${result}`);
      }
    } catch (error) {
      setSdkStatus('Error');
      addResult(`💥 Error: ${error}`);
    }
  };

  const testDataProcessing = async () => {
    const sdk = LeafraSDK.getInstance();
    const testData = [10, 20, 30, 40, 50];
    
    try {
      const result = await sdk.processData(testData);
      if (result.result === ResultCode.SUCCESS) {
        addResult(`📊 Input: [${testData.join(', ')}]`);
        addResult(`📊 Output: [${result.output.join(', ')}]`);
      } else {
        addResult(`❌ Processing failed: ${result.result}`);
      }
    } catch (error) {
      addResult(`💥 Processing error: ${error}`);
    }
  };

  const testMathOperations = async () => {
    const sdk = LeafraSDK.getInstance();
    
    try {
      // Test distance calculation
      const distance = await sdk.calculateDistance2D(
        { x: 0, y: 0 }, 
        { x: 3, y: 4 }
      );
      addResult(`📐 Distance (0,0) to (3,4): ${distance.toFixed(2)}`);
      
      // Test matrix determinant
      const identityMatrix = { 
        data: [1, 0, 0, 0, 1, 0, 0, 0, 1] 
      };
      const det = await sdk.matrixDeterminant(identityMatrix);
      addResult(`🔢 Identity matrix determinant: ${det}`);
      
    } catch (error) {
      addResult(`💥 Math error: ${error}`);
    }
  };

  const addResult = (message: string) => {
    setResults(prev => [...prev.slice(-9), message]); // Keep last 10 results
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>LeafraSDK Example</Text>
      <Text style={styles.status}>Status: {sdkStatus}</Text>
      
      <View style={styles.buttonContainer}>
        <Button title="Test Data Processing" onPress={testDataProcessing} />
        <Button title="Test Math Operations" onPress={testMathOperations} />
      </View>
      
      <View style={styles.resultsContainer}>
        <Text style={styles.resultsTitle}>Results:</Text>
        {results.map((result, index) => (
          <Text key={index} style={styles.resultText}>
            {result}
          </Text>
        ))}
      </View>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    padding: 20,
    backgroundColor: '#f5f5f5',
  },
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    textAlign: 'center',
    marginBottom: 20,
  },
  status: {
    fontSize: 16,
    textAlign: 'center',
    marginBottom: 20,
    color: '#666',
  },
  buttonContainer: {
    flexDirection: 'row',
    justifyContent: 'space-around',
    marginBottom: 20,
  },
  resultsContainer: {
    flex: 1,
    backgroundColor: '#fff',
    padding: 15,
    borderRadius: 8,
  },
  resultsTitle: {
    fontSize: 18,
    fontWeight: 'bold',
    marginBottom: 10,
  },
  resultText: {
    fontSize: 14,
    marginBottom: 5,
    fontFamily: 'monospace',
  },
});

export default LeafraExample;
```

## 🔧 Architecture Overview

### Bridge Architecture

```
React Native (JavaScript/TypeScript)
           ↕
LeafraSDKModule (Objective-C++)
           ↕
LeafraSDKBridge (Objective-C++)
           ↕
LeafraCore (C++)
```

### Key Components

1. **LeafraSDKModule**: React Native module that exposes SDK functionality to JavaScript
2. **LeafraSDKBridge**: Converts between Objective-C and C++ types
3. **LeafraCore**: Core C++ SDK implementation
4. **TypeScript Interface**: Type-safe JavaScript/TypeScript API

### Thread Safety

- All SDK operations are thread-safe
- Heavy operations are automatically dispatched to background queues
- Event callbacks are delivered on the main thread

## 🛠 Build Configuration

### CMake Integration

The SDK uses CMake for cross-platform building. Key files:

- `sdk/CMakeLists.txt`: Main build configuration
- `sdk/react-native/ios/CMakeLists.txt`: iOS-specific settings
- `sdk/corecpp/CMakeLists.txt`: Core C++ library

### CocoaPods Integration

The `LeafraSDK.podspec` file defines:

- Source files and headers
- Dependencies (React-Core)
- Compiler settings (C++17, libc++)
- Platform targets (iOS 15.1+, macOS 12.0+)