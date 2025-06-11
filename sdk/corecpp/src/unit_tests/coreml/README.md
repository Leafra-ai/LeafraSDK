# CoreMLModel Unit Tests

This directory contains comprehensive unit tests for the `CoreMLModel` class, designed to test all public methods and edge cases with a sample model.

## Test Model Specification

The test model (`model.mlmodelc`) has the following characteristics:
- **2 inputs** of size **512 each**
  - Input[0]: `attention_mask` (size: 512)
  - Input[1]: `input_ids` (size: 512)
- **1 output** of size **384**
  - Output[0]: `sentence_embedding` (size: 384)
- **Data type**: Float32

This appears to be a sentence embedding model, likely based on a transformer architecture.

## Test Coverage

The unit tests cover the following functionality:

### 🏗️ **Construction & Initialization**
- ✅ **Basic Model Construction** - Loading model with default settings
- ✅ **Compute Units Configuration** - Testing all compute unit options
- ✅ **Constructor Error Handling** - Invalid model paths and error cases
- ✅ **Move Semantics** - Move constructor and assignment operator

### 🔍 **Model Introspection** 
- ✅ **Input/Output Counts** - Verifying expected input/output structure
- ✅ **Input/Output Sizes** - Checking individual tensor sizes
- ✅ **Input/Output Names** - Retrieving tensor names
- ✅ **Edge Cases** - Out-of-bounds access and invalid indices
- ✅ **Batch Introspection** - Array-based methods for retrieving all metadata
- ✅ **Performance Caching** - Verifying metadata is cached for optimal performance

### 🚀 **Prediction Methods**
- ✅ **Basic Prediction** - Standard prediction with multiple inputs/outputs
- ✅ **Pre-allocated Outputs** - Performance-optimized prediction with pre-allocated buffers
- ✅ **Error Handling** - Wrong input counts, sizes, and validation
- ✅ **Various Input Patterns** - Testing with different data patterns
- ✅ **Performance Testing** - Multiple predictions and timing verification

### 🛡️ **Error Handling & Edge Cases**
- ✅ **Invalid Model Behavior** - Testing behavior after move operations
- ✅ **Input Validation** - Wrong tensor counts and sizes
- ✅ **Exception Safety** - Proper exception handling throughout

## Test Files

### 📋 **test_coreml_model.cpp** - Main Test Suite
Comprehensive unit tests with **14 test cases** covering all CoreMLModel functionality:

1. `test_model_construction` - Basic model loading
2. `test_model_construction_with_compute_units` - All compute unit types
3. `test_model_construction_errors` - Error handling for invalid models
4. `test_move_semantics` - Move constructor and assignment
5. `test_model_introspection` - Basic metadata retrieval
6. `test_introspection_edge_cases` - Out-of-bounds access
7. `test_batch_introspection` - Array-based metadata methods
8. `test_basic_prediction` - Standard prediction workflow
9. `test_prediction_with_preallocated_outputs` - Performance optimization
10. `test_prediction_error_handling` - Input validation
11. `test_prediction_performance` - Performance benchmarking
12. `test_various_input_patterns` - Different data patterns
13. `test_metadata_caching_performance` - Caching verification
14. `test_invalid_model_behavior` - Edge cases after move

## Running the Tests

### Method 1: Quick Test Run

From the `coreml/` directory:

```bash
# Build and run CoreML tests
mkdir build && cd build
cmake ..
make
./test_coreml_model
```

### Method 2: Using CMake Custom Target

From the `unit_tests/` directory:

```bash
mkdir build && cd build
cmake ..
make run_coreml_tests    # Run only CoreML tests
```

### Method 3: Individual Compilation

For quick development testing:

```bash
# From coreml/ directory
g++ -std=c++17 -x objective-c++ \
    -I ../../../include \
    -framework CoreML -framework Foundation \
    -DLEAFRA_HAS_COREML=1 \
    -o test_coreml test_coreml_model.cpp \
    ../../../src/leafra_coreml.mm ../../../src/logger.cpp

./test_coreml
```

## Expected Output

When all tests pass, you should see:

```
=== CoreMLModel Unit Tests ===
Platform: macOS
Model: model.mlmodelc
Expected: 2 inputs (512 each), 1 output (384)

Running test_model_construction... PASS
Running test_model_construction_with_compute_units... PASS
Running test_model_construction_errors... PASS
Running test_move_semantics... PASS
Running test_model_introspection... PASS
Running test_introspection_edge_cases... PASS
Running test_batch_introspection... PASS
Running test_basic_prediction... PASS
Running test_prediction_with_preallocated_outputs... PASS
Running test_prediction_error_handling... PASS
Running test_prediction_performance... (10 predictions in 45ms) PASS
Running test_various_input_patterns... PASS
Running test_metadata_caching_performance... (1000 introspection calls in 234μs) PASS
Running test_invalid_model_behavior... PASS

=== Test Results ===
Total tests: 14
Passed: 14
Failed: 0
Success rate: 100%
🎉 All tests passed!
```

## Performance Benchmarks

The tests include performance verification to ensure optimal operation:

| **Operation** | **Expected Performance** | **Test Verification** |
|---------------|-------------------------|----------------------|
| **Model Loading** | < 1 second | Constructor tests |
| **Prediction** | < 100ms per prediction | Performance test (10 predictions) |
| **Metadata Access** | < 1μs per call | Caching test (1000 calls < 1ms) |
| **Memory Usage** | Minimal allocations | Pre-allocated output tests |

## Prerequisites

- **macOS/iOS**: CoreML framework is required
- **C++17**: Modern C++ features are used
- **Model File**: `model.mlmodelc` must be present in the test directory

## Platform Support

- ✅ **macOS**: Full support with CoreML framework
- ✅ **iOS**: Full support with CoreML framework  
- ❌ **Windows/Linux**: Not supported (CoreML is Apple-only)

## Troubleshooting

### Common Issues

1. **CoreML Framework Not Found**
   ```
   ❌ CoreML framework not found
   ```
   - Ensure you're running on macOS/iOS
   - Check Xcode/iOS SDK installation

2. **Model File Not Found**
   ```
   FAIL - Failed to load CoreML model: The file "model.mlmodelc" couldn't be opened...
   ```
   - Ensure `model.mlmodelc` is in the test directory
   - Check file permissions

3. **Compilation Errors**
   ```
   'leafra/leafra_coreml.h' file not found
   ```
   - Ensure include paths are correct
   - Build from proper directory with CMake

### Debug Tips

- Use `LEAFRA_DEBUG=1` environment variable for verbose logging
- Check model metadata with the introspection tests first
- Verify model compatibility with the expected input/output specification

## Integration with CI/CD

To run CoreML tests in automated environments:

```bash
# Check if CoreML is available before running tests
if [[ "$OSTYPE" == "darwin"* ]]; then
    make run_coreml_tests
else
    echo "Skipping CoreML tests (not on macOS)"
fi
``` 