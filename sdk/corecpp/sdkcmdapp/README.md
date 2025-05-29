# LeafraSDK Command Line Application

## Overview

The **LeafraSDK Command Line Application** (`sdkcmdline`) is an end-to-end testing and development tool for the LeafraSDK. This application allows developers to test the complete SDK functionality from the command line, making SDK development, debugging, and validation significantly easier.

## Purpose

🎯 **End-to-End Testing**: Complete workflow testing from document parsing to chunking  
🔧 **Development Tool**: Rapid SDK feature testing during development  
📝 **Integration Validation**: Verify all SDK components work together correctly  
🚀 **Command Line Interface**: Easy automation and scripting capabilities

> **Note**: This is **NOT a unit test** - it's a comprehensive end-to-end application that exercises the full SDK functionality.

## Supported Platforms

✅ **Desktop Platforms** (Fully Supported):
- **macOS** - Native development and testing platform
- **Linux** - Server and development environments  
- **Windows** - Cross-platform compatibility

❌ **Mobile Platforms** (Not Supported):
- **iOS** - Not needed for command line testing
- **Android** - Not needed for command line testing

*Mobile platform support is intentionally excluded as this is a development/testing tool.*

## Features Tested

The command line application tests the following SDK features:

### 🔍 **Core SDK Functionality**
- SDK initialization and configuration
- Event system and callbacks
- Proper resource management and shutdown

### 📄 **Document Processing**
- Multi-format file parsing (TXT, PDF, DOCX, etc.)
- Page extraction and content analysis
- Metadata extraction and processing

### 🧩 **Advanced Chunking**
- Token-aware text chunking
- Configurable chunk sizes and overlap
- Multiple token approximation methods
- Word boundary preservation

### ⚙️ **Configuration Management**
- Centralized configuration system
- Runtime parameter adjustment
- Performance tuning validation

## Usage

### Basic Usage
```bash
# Build the application
make sdkcmdline

# Run with default settings
./sdkcmdline

# The application will:
# 1. Create a sample document
# 2. Initialize the SDK with chunking enabled
# 3. Process the document with automatic chunking
# 4. Display detailed results and statistics
# 5. Clean up resources properly
```

### Example Output
```
============================================================
  LeafraSDK Command Line Application
============================================================
📄 Created sample document: sample_document.txt

============================================================
  SDK Configuration
============================================================
SDK Name: LeafraSDK-CLI
Chunking Enabled: Yes
Chunk Size: 50 tokens
Overlap: 20%
Token Method: Word-based approximation

============================================================
  Document Processing with Chunking
============================================================
📢 Event: Processing 1 user files
📢 Event: ✅ Parsed Text: sample_document.txt
📢 Event: 🧩 Created 3 chunks
📢 Event: 📊 Chunks: 3, Avg size: 253 chars, 50 tokens

✅ End-to-end processing completed successfully!
```

## Development Benefits

### 🚀 **Accelerated Development**
- **Rapid Testing**: Quickly validate new features without complex setup
- **Integration Verification**: Ensure all components work together
- **Regression Testing**: Detect issues early in development cycle

### 🔧 **Debugging Support**
- **Event Monitoring**: Real-time SDK event tracking
- **Detailed Logging**: Comprehensive debug information
- **Error Detection**: Clear error reporting and diagnosis

### 📊 **Performance Analysis**
- **Timing Information**: Measure processing performance
- **Memory Usage**: Monitor resource consumption
- **Throughput Testing**: Validate processing capabilities

### 🤝 **Team Collaboration**
- **Consistent Testing**: Standardized testing environment
- **Demo Capability**: Easy demonstration of SDK features
- **Documentation**: Living example of SDK usage

## Build Integration

The command line application is integrated into the main CMake build system:

### CMake Target
```cmake
# Automatically built with the main project
add_executable(sdkcmdline sdkcmdapp/sdkcmdline.cpp)
target_link_libraries(sdkcmdline LeafraCore)
target_include_directories(sdkcmdline PRIVATE include)
```

### Build Commands
```bash
# Build everything (includes sdkcmdline)
make

# Build only the command line app
make sdkcmdline

# Clean build
make clean && make
```

## File Structure

```
sdk/corecpp/
├── sdkcmdapp/
│   ├── README.md                    ← This file
│   └── sdkcmdline.cpp              ← Main command line application
├── src/                            ← Core SDK source files
├── include/                        ← SDK headers
├── build/                          ← Build artifacts
└── CMakeLists.txt                  ← Build configuration
```

## Configuration

The application uses a comprehensive configuration setup that demonstrates:

```cpp
// Example configuration from sdkcmdline.cpp
Config config;
config.name = "LeafraSDK-CLI";
config.debug_mode = true;  // Enable detailed logging

// Chunking configuration
config.chunking.enabled = true;
config.chunking.chunk_size = 50;  // 50 tokens per chunk
config.chunking.overlap_percentage = 0.2;  // 20% overlap
config.chunking.size_unit = ChunkSizeUnit::TOKENS;
config.chunking.token_method = TokenApproximationMethod::WORD_BASED;
```

## Extending the Application

To add new testing scenarios:

1. **Modify `sdkcmdline.cpp`** - Add new test functions
2. **Update configuration** - Test different SDK parameters
3. **Add new file types** - Test additional document formats
4. **Enhance reporting** - Add more detailed output

## Troubleshooting

### Common Issues

**Build Failures**:
- Ensure all dependencies are installed
- Check CMake configuration
- Verify C++17 compiler support

**Runtime Errors**:
- Check file permissions
- Verify SDK initialization
- Review log output for details

**Performance Issues**:
- Enable debug mode for detailed timing
- Check memory usage
- Validate chunking parameters

## Future Enhancements

- **Command Line Arguments**: Accept file paths and configuration parameters
- **Batch Processing**: Support multiple files and directories
- **Output Formats**: JSON, XML, or CSV result output
- **Performance Benchmarking**: Automated performance testing
- **Configuration Files**: External configuration file support

---

**This command line application is an essential development tool that makes LeafraSDK development faster, more reliable, and easier to debug.** 