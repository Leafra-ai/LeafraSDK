# Makefile for building test_model.cpp for Apple platforms
# Uses prebuilt ExecutorTorch xcframeworks

# Compiler settings
CXX = clang++

# Debug mode (set DEBUG=1 to enable)
DEBUG ?= 0

ifeq ($(DEBUG),1)
    CXXFLAGS = -std=c++17 -Wall -Wextra -g -O0 -DDEBUG
    BUILD_TYPE = debug
else
    CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -DNDEBUG
    BUILD_TYPE = release
endif

# Determine the platform
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S),Darwin)
    ifeq ($(UNAME_M),arm64)
        PLATFORM = macos-arm64
    else
        PLATFORM = macos-x86_64
    endif
else
    $(error This Makefile is designed for Apple platforms only)
endif

# Base path to prebuilt executorch
EXECUTORCH_BASE = ../../prebuilt/executorch/apple

# Header paths from xcframeworks (only executorch has headers)
EXECUTORCH_HEADERS = $(EXECUTORCH_BASE)/executorch.xcframework/$(PLATFORM)/Headers

# Include paths  
INCLUDES = -I$(EXECUTORCH_HEADERS)

# Additional compiler flags to handle missing symbols and API mismatches
CXXFLAGS += -Wno-error=unused-parameter -Wno-error=unused-variable -fpermissive

# Library paths
EXECUTORCH_LIB = $(EXECUTORCH_BASE)/executorch.xcframework/$(PLATFORM)/libexecutorch_$(shell echo $(PLATFORM) | cut -d'-' -f1).a
KERNELS_PORTABLE_LIB = $(EXECUTORCH_BASE)/kernels_portable.xcframework/$(PLATFORM)/libkernels_portable_$(shell echo $(PLATFORM) | cut -d'-' -f1).a
KERNELS_OPTIMIZED_LIB = $(EXECUTORCH_BASE)/kernels_optimized.xcframework/$(PLATFORM)/libkernels_optimized_$(shell echo $(PLATFORM) | cut -d'-' -f1).a

# Libraries to link
# Use -Wl,-force_load for portable kernels to ensure all symbols are included
# (macOS equivalent of -Wl,--whole-archive on Linux)
LIBS = $(EXECUTORCH_LIB) -Wl,-force_load,$(KERNELS_PORTABLE_LIB) -Wl,-force_load,$(KERNELS_OPTIMIZED_LIB)

# Target executable
TARGET = test_model
SOURCE = test_model.cpp

# Default target
all: $(TARGET)

# Debug target for convenience
debug:
	$(MAKE) DEBUG=1

# Build the target
$(TARGET): $(SOURCE)
	@echo "Building $(TARGET) in $(BUILD_TYPE) mode..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(SOURCE) $(LIBS)

# Clean target
clean:
	rm -f $(TARGET)

# Check if required libraries exist
check:
	@echo "Checking for required libraries..."
	@echo "Platform: $(PLATFORM)"
	@echo "ExecutorTorch lib: $(EXECUTORCH_LIB)"
	@if [ -f "$(EXECUTORCH_LIB)" ]; then echo "✅ ExecutorTorch library found"; else echo "❌ ExecutorTorch library not found"; fi
	@echo "Kernels portable lib: $(KERNELS_PORTABLE_LIB)"
	@if [ -f "$(KERNELS_PORTABLE_LIB)" ]; then echo "✅ Kernels portable library found"; else echo "❌ Kernels portable library not found"; fi
	@echo "Kernels optimized lib: $(KERNELS_OPTIMIZED_LIB)"
	@if [ -f "$(KERNELS_OPTIMIZED_LIB)" ]; then echo "✅ Kernels optimized library found"; else echo "❌ Kernels optimized library not found"; fi
	@echo "Headers: $(EXECUTORCH_HEADERS)"
	@if [ -d "$(EXECUTORCH_HEADERS)" ]; then echo "✅ Headers directory found"; else echo "❌ Headers directory not found"; fi
	@echo "Available header directories:"
	@find $(EXECUTORCH_HEADERS) -type d -name "*" | head -10

# Install target (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Build the test_model executable"
	@echo "  clean   - Remove built files"
	@echo "  check   - Check if required libraries and headers exist"
	@echo "  install - Install the executable to /usr/local/bin"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Build modes:"
	@echo "  make           - Build in release mode (optimized, -O2)"
	@echo "  make DEBUG=1   - Build in debug mode (debug symbols, -g -O0)"
	@echo ""
	@echo "Kernel options:"
	@echo "  make USE_OPTIMIZED=0  - Use portable kernels only (default, stable)"
	@echo "  make USE_OPTIMIZED=1  - Use optimized kernels (experimental, may crash)"
	@echo ""
	@echo "Current build mode: $(BUILD_TYPE)"
	@echo "Current kernel mode: $(if $(filter 1,$(USE_OPTIMIZED)),optimized,portable)"

.PHONY: all debug clean check install help