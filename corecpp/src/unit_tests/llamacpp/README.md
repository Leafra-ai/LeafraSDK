# LlamaCpp Unit Tests

This directory contains comprehensive unit tests for the LlamaCpp model integration in LeafraSDK.

## Overview

The unit tests cover all public APIs of the `LlamaCppModel` class, including:

- **Model Loading & State Management**: Loading models, checking state, unloading
- **Tokenization & Detokenization**: Converting text to tokens and back
- **Text Generation**: Synchronous text generation with various prompts
- **Streaming Generation**: Asynchronous text generation with callbacks
- **Chat Templates**: Conversation formatting and chat response generation
- **Context Management**: Context usage tracking and reset functionality
- **Configuration**: Parameter access and updates
- **Error Handling**: Graceful handling of invalid inputs and states

## Test Implementation

The tests are implemented using a simple custom testing framework (no external dependencies like Google Test) to ensure compatibility and ease of building.

### Files

- `test_llamacpp_simple.cpp` - Main test implementation with custom assertion macros
- `CMakeLists.txt` - Build configuration
- `README.md` - This documentation

## Model Requirements

The tests use the Llama-3.2-3B-Instruct model located at:
```
corecpp/third_party/models/llm/unsloth/Llama-3.2-3B-Instruct-Q4_K_M.gguf
```

Make sure this model file exists before running the tests.

## Building the Tests

### Prerequisites

- CMake 3.18 or higher
- C++17 compatible compiler
- LlamaCpp library properly built and linked

### Build Steps

1. **Navigate to the build directory:**
   ```bash
   cd sdk/build/macos  # or your build directory
   ```

2. **Configure with CMake:**
   ```bash
   cmake ../..
   ```

3. **Build the tests:**
   ```bash
   make test_llamacpp_simple
   ```

4. **Or build and run directly:**
   ```bash
   make run_llamacpp_tests
   ```

## Running the Tests

### Method 1: Using Make Target
```bash
make run_llamacpp_tests
```

### Method 2: Direct Execution
```bash
./corecpp/src/unit_tests/llamacpp/test_llamacpp_simple
```

### Method 3: From Test Directory
```bash
cd corecpp/src/unit_tests/llamacpp
./test_llamacpp_simple
```

## Test Configuration

The tests use the following default configuration:

```cpp
config_.n_ctx = 2048;           // Context size
config_.n_predict = 50;         // Max tokens to generate
config_.temperature = 0.7f;     // Sampling temperature
config_.top_p = 0.9f;          // Nucleus sampling
config_.top_k = 40;            // Top-k sampling
config_.n_threads = 4;         // Number of threads
config_.debug_mode = false;    // Debug output disabled
```

## Expected Output

When all tests pass, you should see output like:

```
=== LlamaCpp Model Unit Tests ===
Testing model loading and state...
✅ Model loading and state test passed
Testing tokenization and detokenization...
✅ Tokenization and detokenization test passed
Testing basic text generation...
   Generated: Paris, the capital and largest city of France.
✅ Basic text generation test passed
Testing streaming text generation...
   Generated: , there was a young girl named
✅ Streaming text generation test passed
Testing chat template functionality...
   Chat response: 2+2 equals 4.
✅ Chat template functionality test passed
Testing context management...
✅ Context management test passed
Testing configuration access...
✅ Configuration access test passed
Testing error handling...
✅ Error handling test passed

=== Test Results ===
Passed: 8/8
🎉 All tests passed!
```

## Debug Mode

To build and run tests in debug mode with additional diagnostics:

```bash
DEBUG=1 cmake ../..
make run_llamacpp_tests
```

Debug mode enables:
- AddressSanitizer for memory error detection
- Maximum debug symbols (-g3)
- Disabled optimizations (-O0)
- Additional debug output

## Troubleshooting

### Model Not Found
If you see "Failed to load model" errors, ensure:
1. The model file exists at the specified path
2. The model file is not corrupted
3. You have read permissions for the model file

### Build Failures
If compilation fails:
1. Ensure LlamaCpp headers are available in `corecpp/include/leafra/`
2. Verify LlamaCpp source files exist in `corecpp/src/`
3. Check that LEAFRA_HAS_LLAMACPP is properly defined

### Runtime Errors
If tests crash or fail:
1. Try running in debug mode for better error information
2. Check that llama.cpp library is properly initialized
3. Verify model compatibility with the current llama.cpp version

## Adding New Tests

To add new test cases:

1. Add a new test method to the `LlamaCppTester` class
2. Follow the naming convention: `test_<functionality_name>()`
3. Use the provided assertion macros: `ASSERT_TRUE`, `ASSERT_FALSE`, `ASSERT_EQ`, etc.
4. Add the test to the `run_all_tests()` method
5. Document the new test in this README

## Integration with CI/CD

These tests can be integrated into continuous integration pipelines:

```bash
# Build tests
cmake --build . --target test_llamacpp_simple

# Run tests (returns 0 on success, 1 on failure)
./corecpp/src/unit_tests/llamacpp/test_llamacpp_simple
```

The test executable returns appropriate exit codes for automated testing systems. 