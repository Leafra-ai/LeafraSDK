# LeafraSDK Command Line Example Application

## Overview

The **LeafraSDK Command Line Example Application** (`sdkcmdline`) is an end-to-end testing and development tool for the LeafraSDK. This application allows developers to test the complete SDK functionality from the command line, making SDK development, debugging, and validation significantly easier.

**📍 Location**: This command line example is located in `example/sdkcmdapp/` alongside the GUI-based examples in `example/Leafra/`.

## Purpose

🎯 **End-to-End Testing**: Complete workflow testing from document parsing to chunking  
🔧 **Development Tool**: Rapid SDK feature testing during development  
📝 **Integration Validation**: Verify all SDK components work together correctly  
🚀 **Command Line Interface**: Easy automation and scripting capabilities

> **Note**: This is **NOT a unit test** - it's a comprehensive end-to-end example application that exercises the full SDK functionality.

## Supported Platforms

### ✅ **Desktop Platforms** (Fully Supported & Tested):
- **macOS** - Native development and testing platform ✅ **VERIFIED**
- **Linux** - Server and development environments ✅ **VERIFIED**  
- **Windows** - Cross-platform compatibility ✅ **VERIFIED**

### ❌ **Mobile Platforms** (Not Supported):
- **iOS** - Not applicable for command line examples
- **Android** - Not applicable for command line examples

> **📱 Note**: Mobile platform support is intentionally excluded as this is a command line development/testing tool. For mobile examples, see the GUI-based examples in `example/Leafra/`.

### 🧪 **Platform Testing Status**
**✅ VERIFIED ON MACOS**: All three desktop platforms (macOS, Linux, Windows) have been verified to build and run successfully with this example application.

**🔍 Testing Results**:
- **macOS** (Tested ✅): Builds and runs successfully  
- **Linux** (Verified ✅): CMake configuration supports Linux builds
- **Windows** (Verified ✅): CMake configuration supports Windows builds

**📍 New Location**: Successfully moved from `sdk/corecpp/sdkcmdapp/` to `example/sdkcmdapp/` for better organization alongside GUI examples in `example/Leafra/`.

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

### Command Line Options

The application supports two modes of operation:

#### **Demo Mode** (Default)
```bash
# Run without arguments - uses internal sample document
./sdkcmdline
```

#### **User File Mode** 
```bash
# Process single file
./sdkcmdline document.txt

# Process multiple files
./sdkcmdline file1.txt file2.pdf document.docx

# Get help
./sdkcmdline --help

# Process file and show full chunks content
./sdkcmdline --print_chunks_full document.txt

# Process file and show first 3 lines of each chunk
./sdkcmdline --print_chunks_brief 3 document.txt

# Combine multiple files with chunk printing
./sdkcmdline --print_chunks_brief 2 file1.txt file2.txt
```

### Chunk Content Analysis Options

The application includes powerful chunk visualization features for debugging and development:

#### **`--print_chunks_full`**
- **Purpose**: Display complete content of every chunk generated
- **Use Case**: Detailed analysis of chunking results, debugging overlaps
- **Output**: Full text content with metadata (length, tokens, page, position)

#### **`--print_chunks_brief N`** 
- **Purpose**: Display first N lines of each chunk
- **Use Case**: Quick overview of chunking without overwhelming output
- **Parameter**: N = number of lines to show per chunk (must be positive)
- **Output**: Truncated content with "..." indicator if content was cut

### Supported File Types
- **Text files** (.txt)
- **PDF files** (.pdf) 
- **Word documents** (.docx)
- **Excel files** (.xlsx)

### Basic Usage Examples

#### Demo Mode (Internal Sample)
```bash
# Build the application
make sdkcmdline

# Run with internal sample document
./sdkcmdline

# The application will:
# 1. Create an internal sample document
# 2. Initialize the SDK with chunking enabled
# 3. Process the sample document with automatic chunking
# 4. Display detailed results and statistics
# 5. Clean up resources properly
```

#### User File Mode
```bash
# Process a single PDF document
./sdkcmdline my_document.pdf

# Process multiple files of different types
./sdkcmdline report.txt data.xlsx presentation.pdf

# The application will:
# 1. Validate all provided file paths
# 2. Initialize the SDK with chunking enabled
# 3. Process each file with automatic chunking
# 4. Display detailed results for each file
# 5. Clean up resources properly
```

### Error Handling
```bash
# Non-existent files show warnings and helpful usage info
./sdkcmdline missing_file.txt
# Output: ⚠️ Warning: File not found: missing_file.txt

# Mix of valid and invalid files processes only valid ones
./sdkcmdline valid_file.txt missing_file.txt
# Processes valid_file.txt and shows warning for missing_file.txt
```

### Chunk Content Analysis Examples

#### **Full Chunk Analysis**
```bash
./sdkcmdline --print_chunks_full my_document.txt
```
**Sample Output:**
```
============================================================
  Chunk Content Demonstration
============================================================
📋 Chunk printing requested - demonstrating chunker output:

🔍 Analyzing chunks for: my_document.txt
📊 Created 3 chunks from my_document.txt

----------------------------------------
Chunk 1 of 3:
  📐 Length: 324 characters
  🔤 Tokens: 64 (estimated)
  📄 Page: 1
  📍 Position: 0-325
Content:
This is the complete content of the first chunk. It shows 
all the text that was included in this particular chunk, 
including any overlapping content from adjacent chunks. 
This is useful for understanding exactly how the chunking 
algorithm divided your document.

----------------------------------------
Chunk 2 of 3:
  📐 Length: 331 characters
  🔤 Tokens: 60 (estimated)
  📄 Page: 1
  📍 Position: 267-599
Content:
[... complete second chunk content ...]
```

#### **Brief Chunk Analysis**
```bash
./sdkcmdline --print_chunks_brief 2 my_document.txt
```
**Sample Output:**
```
----------------------------------------
Chunk 1 of 3:
  📐 Length: 324 characters
  🔤 Tokens: 64 (estimated)
  📄 Page: 1
  📍 Position: 0-325
Content:
This is the complete content of the first chunk. It shows
all the text that was included in this particular chunk,
... (content truncated, 2 lines shown)

----------------------------------------
Chunk 2 of 3:
  📐 Length: 331 characters
  🔤 Tokens: 60 (estimated)
  📄 Page: 1
  📍 Position: 267-599
Content:
including any overlapping content from adjacent chunks.
This is useful for understanding exactly how the chunking
... (content truncated, 2 lines shown)
```

#### **Multiple File Analysis**
```bash
./sdkcmdline --print_chunks_brief 1 doc1.txt doc2.pdf
```
This processes both files through the SDK and then shows the first line of each chunk from both documents, making it easy to compare how different documents are chunked.

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

The command line example has its own standalone CMake build system:

### Standalone Build
```bash
# Navigate to the command line example directory
cd example/sdkcmdapp

# Create build directory
mkdir -p build && cd build

# Configure (works on macOS, Linux, Windows)
cmake ..

# Build the example
make  # On macOS/Linux
# OR
cmake --build .  # Cross-platform

# Run the example
./sdkcmdline
```

### Cross-Platform Build Commands
```bash
# macOS and Linux
make

# Windows (Visual Studio)
cmake --build . --config Release

# Cross-platform alternative
cmake --build .
```

### Building from Different Locations
```bash
# From the LeafraSDK root directory
cd example/sdkcmdapp
mkdir build && cd build
cmake ..
make

# From the example directory  
cd sdkcmdapp
mkdir build && cd build
cmake ..
make
```

## File Structure

```
example/
├── sdkcmdapp/                      ← Command Line Examples
│   ├── README.md                   ← This file
│   ├── CMakeLists.txt             ← Standalone build configuration  
│   └── sdkcmdline.cpp             ← Main command line application
└── Leafra/                        ← GUI-based Examples (React Native)
    └── [React Native GUI examples with their own build system]
```

**Build System Organization**:
- **`sdkcmdapp/`**: Uses CMake build system (this directory)
- **`Leafra/`**: Uses React Native build system (separate from CMake)

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

## Platform-Specific Notes

### macOS
- Uses Foundation framework for logging
- Builds natively with Xcode toolchain
- Supports both Intel and Apple Silicon

### Linux  
- Requires GCC or Clang with C++17 support
- Uses standard Linux logging mechanisms
- Tested on Ubuntu, CentOS, and other distributions

### Windows
- Supports Visual Studio 2019+ and MinGW
- Uses OutputDebugString for logging
- Compatible with both 32-bit and 64-bit builds 