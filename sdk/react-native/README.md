# react-native-leafra-sdk

React Native bindings for LeafraSDK - A high-performance cross-platform C++ library for data processing, mathematical operations, document parsing, semantic search, and AI/ML inference capabilities.

## Features

- 📄 **Document parsing**: PDF, DOCX, TXT, Excel files
- 🧮 **Mathematical operations**: Matrix operations, distance calculations, determinants
- 🔍 **Semantic search**: Vector-based search with AI embeddings
- 🤖 **AI/ML inference**: CoreML and LlamaCpp integration
- 🚀 **High performance**: Native C++ core with React Native bindings
- 📱 **Cross-platform**: iOS and macOS support

## Installation

### Prerequisites

1. **Install the React Native package**:
```bash
npm install react-native-leafra-sdk
# or
yarn add react-native-leafra-sdk
```

**Note**: The frameworks are automatically copied to the react-native package during the SDK build process.

### iOS Setup

1. **Add to your Podfile**:
```ruby
pod 'react-native-leafra-sdk', :path => '../node_modules/react-native-leafra-sdk'
```

2. **Install pods**:
```bash
cd ios && pod install
```

3. **Framework Integration**: The podspec automatically includes:
   - `ios/LeafraCore.framework` (main SDK for iOS)
   - `ios/llama.framework` (LLM support for iOS)
   - `ios/LeafraCore.framework/LeafraResources.bundle` (bundled models)

### macOS Setup

Similar to iOS, but for macOS target in your Podfile:
```ruby
target 'YourMacOSApp' do
  platform :osx, '10.15'
  pod 'react-native-leafra-sdk', :path => '../node_modules/react-native-leafra-sdk'
end
```

**Framework Integration**: The podspec automatically includes:
- `macos/LeafraCore.framework` (main SDK for macOS)
- `macos/llama.framework` (LLM support for macOS)
- `macos/LeafraCore.framework/LeafraResources.bundle` (bundled models)

## Usage

### Import the SDK

```typescript
import LeafraSDK from 'react-native-leafra-sdk';
```

### Initialize the SDK

```typescript
const config = {
  // Configuration options
  logLevel: 'info',
  enableDebug: true
};

try {
  const result = await LeafraSDK.initialize(config);
  console.log('SDK initialized:', result);
} catch (error) {
  console.error('Failed to initialize SDK:', error);
}
```

### Document Processing

```typescript
// Process user files
const fileUrls = [
  'file:///path/to/document.pdf',
  'file:///path/to/spreadsheet.xlsx'
];

try {
  const result = await LeafraSDK.processUserFiles(fileUrls);
  console.log('Processing result:', result);
} catch (error) {
  console.error('Processing failed:', error);
}
```

### Mathematical Operations

```typescript
// Calculate distance between 2D points
const point1 = { x: 0, y: 0 };
const point2 = { x: 3, y: 4 };

const distance = await LeafraSDK.calculateDistance2D(point1, point2);
console.log('Distance:', distance); // 5.0

// Matrix operations
const matrixA = { data: [1, 2, 3, 4, 5, 6, 7, 8, 9] }; // 3x3 matrix
const matrixB = { data: [9, 8, 7, 6, 5, 4, 3, 2, 1] }; // 3x3 matrix

const result = await LeafraSDK.multiplyMatrix3x3(matrixA, matrixB);
console.log('Matrix multiplication result:', result);
```

### Semantic Search

```typescript
// Basic semantic search
const query = "artificial intelligence machine learning";
const maxResults = 10;

try {
  const results = await LeafraSDK.semanticSearch(query, maxResults);
  console.log('Search results:', results);
} catch (error) {
  console.error('Search failed:', error);
}

// Semantic search with LLM
try {
  const results = await LeafraSDK.semanticSearchWithLLM(query, maxResults);
  console.log('LLM search results:', results);
} catch (error) {
  console.error('LLM search failed:', error);
}
```

### Event Handling

```typescript
// Listen to SDK events
LeafraSDK.addListener('onProgress', (event) => {
  console.log('Progress:', event.progress);
});

LeafraSDK.addListener('onError', (event) => {
  console.error('SDK Error:', event.error);
});

LeafraSDK.addListener('onComplete', (event) => {
  console.log('Operation completed:', event.result);
});
```

### Cleanup

```typescript
// Shutdown the SDK when done
try {
  await LeafraSDK.shutdown();
  console.log('SDK shutdown successfully');
} catch (error) {
  console.error('Shutdown failed:', error);
}
```

## API Reference

### Core Methods

| Method | Description | Parameters | Returns |
|--------|-------------|------------|---------|
| `initialize(config)` | Initialize the SDK | `config: object` | `Promise<number>` |
| `shutdown()` | Shutdown the SDK | - | `Promise<number>` |
| `isInitialized()` | Check if SDK is initialized | - | `Promise<boolean>` |
| `getVersion()` | Get SDK version | - | `Promise<string>` |
| `getPlatform()` | Get platform info | - | `Promise<string>` |

### Document Processing

| Method | Description | Parameters | Returns |
|--------|-------------|------------|---------|
| `processUserFiles(fileUrls)` | Process documents | `fileUrls: string[]` | `Promise<object>` |
| `processData(input)` | Process numerical data | `input: number[]` | `Promise<object>` |

### Mathematical Operations

| Method | Description | Parameters | Returns |
|--------|-------------|------------|---------|
| `calculateDistance2D(p1, p2)` | 2D distance calculation | `p1, p2: {x, y}` | `Promise<number>` |
| `calculateDistance3D(p1, p2)` | 3D distance calculation | `p1, p2: {x, y, z}` | `Promise<number>` |
| `multiplyMatrix3x3(a, b)` | 3x3 matrix multiplication | `a, b: {data: number[]}` | `Promise<object>` |
| `matrixDeterminant(matrix)` | Matrix determinant | `matrix: {data: number[]}` | `Promise<number>` |

### Semantic Search

| Method | Description | Parameters | Returns |
|--------|-------------|------------|---------|
| `semanticSearch(query, maxResults)` | Vector-based search | `query: string, maxResults: number` | `Promise<object>` |
| `semanticSearchWithLLM(query, maxResults)` | LLM-enhanced search | `query: string, maxResults: number` | `Promise<object>` |

## Framework Dependencies

The React Native bridge automatically includes these frameworks:

### iOS/macOS Frameworks
- `LeafraCore.framework` - Main SDK functionality
- `llama.framework` - LLM inference support
- `CoreML.framework` - Apple ML framework
- `Foundation.framework` - Base system framework
- `CoreGraphics.framework` - Graphics support
- `Accelerate.framework` - High-performance math

### System Libraries
- `libsqlite3` - Database support
- `libicucore` - Unicode/internationalization

### Resource Bundles
- `LeafraResources.bundle` - Pre-trained models and configurations

## Requirements

- **iOS**: 13.0+
- **macOS**: 10.15+
- **React Native**: 0.60+
- **Xcode**: 12.0+

## Troubleshooting

### Common Issues

1. **Framework not found**: Ensure the package includes `ios/LeafraCore.framework` and `macos/LeafraCore.framework`
2. **Missing models**: Check that LeafraResources.bundle is included in your app bundle
3. **Linker errors**: Verify all system frameworks are properly linked
4. **Platform mismatch**: Ensure you're using the correct framework for your target platform (iOS vs macOS)

### Debug Tips

```typescript
// Enable debug logging
const config = {
  logLevel: 'debug',
  enableDebug: true
};

// Check SDK status
const isInitialized = await LeafraSDK.isInitialized();
const version = await LeafraSDK.getVersion();
const platform = await LeafraSDK.getPlatform();

console.log('SDK Status:', { isInitialized, version, platform });
```

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Support

For issues and questions:
- 📧 Email: support@leafrasdk.com
- 🐛 Issues: [GitHub Issues](https://github.com/your-org/LeafraSDK/issues)
- 📖 Documentation: [LeafraSDK Docs](https://docs.leafrasdk.com) 