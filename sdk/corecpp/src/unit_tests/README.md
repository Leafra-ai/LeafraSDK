# LeafraChunker Unit Tests

This directory contains comprehensive unit tests and demonstration programs for the `LeafraChunker` class, designed to work across all supported platforms: iOS, macOS, Windows, Linux, and Android.

## Test Files Overview

### 📋 **chunker/test_leafra_chunker.cpp** - Main Unit Test Suite
Comprehensive unit tests with **21 test cases** covering all chunker functionality.

### 🎯 **chunker/test_token_chunking.cpp** - Token Chunking Demonstration  
Interactive demonstration of token-based chunking features including:
- Token estimation with three approximation methods
- Token chunking with different chunk sizes
- Multi-page token chunking examples

### 🔧 **chunker/test_unified_api.cpp** - Unified API Demonstration
Shows the clean, non-redundant API design:
- Character vs token chunking using same methods
- Unified `chunk_text_advanced()` and `chunk_document_advanced()` methods
- Legacy method compatibility

### 📚 **../TOKEN_CHUNKING_README.md** - Token Implementation Documentation
Complete documentation for token-based chunking features (located in src/ directory).

## Test Coverage

The unit tests cover the following functionality:

### Core Features
- ✅ **Chunker Initialization** - Basic setup and default configuration
- ✅ **Single Text Chunking** - Basic text splitting with various sizes
- ✅ **Multi-page Document Chunking** - Handling documents with multiple pages
- ✅ **Advanced Chunking Options** - Custom configuration and options
- ✅ **Word Boundary Preservation** - Intelligent word splitting
- ✅ **Statistics Tracking** - Chunk count and character tracking
- ✅ **Options Management** - Default settings and customization

### Token-Based Features ⭐ **NEW**
- ✅ **Token Estimation** - Three approximation methods (Simple, Word-based, Advanced)
- ✅ **Token Chunking** - Size specification in tokens vs characters
- ✅ **Token Conversion** - Bidirectional token ↔ character conversion
- ✅ **Multi-page Token Chunking** - Token-based chunking across pages
- ✅ **Approximation Method Comparison** - Testing accuracy of different methods

### Error Handling
- ✅ **Invalid Parameters** - Empty text, zero chunk size, invalid overlap
- ✅ **Edge Cases** - Very long words, whitespace-only text, Unicode characters
- ✅ **Metadata Consistency** - Proper chunk indexing and page numbers
- ✅ **Token Error Handling** - Validation for token-specific parameters

### Performance
- ✅ **Large Document Processing** - Handling documents with 1000+ words

## Running the Tests

### Method 1: Quick Individual Tests

From the `src/unit_tests/chunker/` directory:

```bash
# Main unit test suite (21 comprehensive tests)
g++ -std=c++17 -I ../../../include -o test test_leafra_chunker.cpp ../../leafra_chunker.cpp
./test

# Token chunking demonstration
g++ -std=c++17 -I ../../../include -o demo_token test_token_chunking.cpp ../../leafra_chunker.cpp
./demo_token

# Unified API demonstration  
g++ -std=c++17 -I ../../../include -o demo_api test_unified_api.cpp ../../leafra_chunker.cpp
./demo_api
```

### Method 2: Using CMake (Recommended)

From the unit_tests directory:

```bash
mkdir build && cd build
cmake ..
make

# Run different targets (executables are in chunker/ subdirectory)
cd chunker
./leafra_chunker_tests    # Main unit tests
./test_token_chunking     # Token demo
./test_unified_api        # API demo

# Or use CMake targets from build directory
cd ..
make run_chunker_tests    # Run main tests
make run_token_demo       # Run token demo
make run_unified_demo     # Run API demo
```

### Method 3: Platform-Specific Builds

#### iOS
```bash
cmake .. -G Xcode -DCMAKE_SYSTEM_NAME=iOS
xcodebuild -project LeafraChunkerTests.xcodeproj -scheme leafra_chunker_tests
```

#### Android
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
         -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-21
make
```

#### Windows (Visual Studio)
```bash
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
Release\leafra_chunker_tests.exe
```

## Expected Output

### Main Unit Test Suite (`test_leafra_chunker.cpp`)

When all tests pass, you should see:

```
=== LeafraChunker Unit Tests ===
Platform: macOS

Running test_chunker_initialization... PASS
Running test_single_text_chunking_basic... PASS
Running test_single_text_chunking_small_text... PASS
Running test_single_text_chunking_overlap... PASS
Running test_multi_page_document_chunking... PASS
Running test_multi_page_single_page... PASS
Running test_advanced_chunking_options... PASS
Running test_word_boundary_preservation... PASS
Running test_statistics_tracking... PASS
Running test_default_options_management... PASS
Running test_error_handling_invalid_parameters... PASS
Running test_edge_case_very_long_words... PASS
Running test_edge_case_only_whitespace... PASS
Running test_edge_case_unicode_text... PASS
Running test_chunk_metadata_consistency... PASS
Running test_performance_large_document... PASS
Running test_token_estimation... PASS
Running test_basic_token_chunking... PASS
Running test_token_multipage_chunking... PASS
Running test_token_chunking_error_handling... PASS
Running test_approximation_methods_comparison... PASS

=== Test Results ===
Total tests: 21
Passed: 21
Failed: 0
Success rate: 100%
🎉 All tests passed!
```

### Token Chunking Demo (`test_token_chunking.cpp`)

Sample output showing token estimation and chunking:

```
Testing LeafraChunker Token-Based Functionality
=============================================

Token Estimation Tests
====================
Test text: "This is a sample text with several words for testing token estimation."
Length: 70 characters

Token estimates:
  Simple (char/4):    18 tokens
  Word-based (×1.33): 16 tokens
  Advanced heuristic: 16 tokens

--- Testing 10 tokens per chunk ---
Simple method:
  Created 3 chunks
  Chunk 1 (9 tokens, 40 chars): "This is a sample text with several words..."
```

### Unified API Demo (`test_unified_api.cpp`)

Sample output demonstrating the unified API:

```
=== Testing Unified API (No Redundancy) ===

1. Character-based chunking (using chunk_text_advanced):
   ✅ Success! Created 2 chunks
   Chunk 1: 87 chars, 0 tokens
   Chunk 2: 78 chars, 0 tokens

2. Token-based chunking (using chunk_text_advanced):
   ✅ Success! Created 2 chunks
   Chunk 1: 133 chars, 31 tokens
   Chunk 2: 48 chars, 11 tokens

🎉 All tests completed successfully!
```

## Test Details

### Sample Test Cases

#### Basic Chunking
- **Input**: "This is a test text that should be chunked into smaller pieces for testing purposes."
- **Chunk Size**: 30 characters
- **Overlap**: 20%
- **Expected**: Multiple chunks with proper overlap

#### Multi-page Document
- **Input**: 3 pages of text
- **Chunk Size**: 80 characters
- **Overlap**: 10%
- **Expected**: Proper page number assignment and cross-page chunking

#### Error Handling
- **Empty text** → `ERROR_INVALID_PARAMETER`
- **Zero chunk size** → `ERROR_INVALID_PARAMETER`
- **Invalid overlap (< 0 or >= 1.0)** → `ERROR_INVALID_PARAMETER`

#### Edge Cases
- **Very long words** (200+ characters) → Handled gracefully
- **Unicode text** (café, résumé, etc.) → Proper character handling
- **Whitespace-only text** → Handled without crashes

## Platform Compatibility

The tests are designed to work on:

- ✅ **macOS** (tested)
- ✅ **iOS** (cross-compilation ready)
- ✅ **Windows** (MinGW and MSVC)
- ✅ **Linux** (GCC and Clang)
- ✅ **Android** (NDK ready)

## Adding New Tests

To add new test cases:

1. Create a new test function following the pattern:
   ```cpp
   bool test_your_feature() {
       LeafraChunker chunker;
       chunker.initialize();
       
       // Your test logic here
       TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Description");
       
       return true;
   }
   ```

2. Add the test to the main function:
   ```cpp
   RUN_TEST(test_your_feature);
   ```

3. Update this README with the new test coverage.

## Troubleshooting

### Common Issues

1. **Compilation errors**: Ensure C++11 support and correct include paths
2. **Platform detection**: Check that platform macros are correctly defined
3. **Unicode issues**: Some platforms may have different Unicode handling

### Debug Mode

To run tests with more verbose output, modify the test macros or add debug prints as needed. 