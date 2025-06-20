# Windows x64 LlamaCpp Libraries

Place the following files in this directory:

- `llama.dll` - Main LlamaCpp dynamic library
- `llama.lib` - Import library for linking

## Build Requirements

When building for Windows x64, CMake will look for libraries in this directory.

## Usage

The build system will automatically detect and link these libraries when building for Windows. 