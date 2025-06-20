# Android ARM64-v8a LlamaCpp Libraries

Place the following files in this directory:

- `libllama.so` - Main LlamaCpp shared library
- `include/` - Directory containing LlamaCpp header files
  - `llama.h`
  - `ggml.h`
  - Other required headers

## Build Requirements

When building for Android ARM64-v8a, CMake will look for:
- Library: `libllama.so` in this directory
- Headers: `include/` subdirectory in this directory

## Usage

The build system will automatically detect and link these libraries when building for Android. 