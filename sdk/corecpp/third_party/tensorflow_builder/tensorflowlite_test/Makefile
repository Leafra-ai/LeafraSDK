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
        PLATFORM = ios-arm64
    else
        PLATFORM = ios-x86_64-simulator
    endif
else
    $(error This Makefile is designed for Apple platforms only)
endif

# Base path to prebuilt executorch
TENSORFLOW_BASE = ../../prebuilt/tensorflowlite/apple
#/Users/arifdikici/Documents/Squirrel/LeafraSDK/sdk/corecpp/third_party/prebuilt/tensorflowlite/apple/TensorFlowLiteC.xcframework/ios-arm64/TensorFlowLiteC.framework/Headers
# Header paths from xcframeworks (only executorch has headers)
TENSORFLOW_HEADERS = $(TENSORFLOW_BASE)/TensorFlowLiteC.xcframework/$(PLATFORM)/TensorFlowLiteC.framework/Headers

# Include paths  
INCLUDES = -I$(TENSORFLOW_HEADERS)

# Additional compiler flags to handle missing symbols and API mismatches
CXXFLAGS += -Wno-error=unused-parameter -Wno-error=unused-variable -fpermissive

# Library paths
TENSORFLOW_LIB = $(TENSORFLOW_BASE)/TensorFlowLiteC.xcframework/$(PLATFORM)/libTensorFlowLiteC_$(shell echo $(PLATFORM) | cut -d'-' -f1).a
TENSORFLOW_COREML_LIB = $(TENSORFLOW_BASE)/TensorFlowLiteCCoreML.xcframework/$(PLATFORM)/libTensorFlowLiteCCoreML_$(shell echo $(PLATFORM) | cut -d'-' -f1).a
TENSORFLOW_METAL_LIB = $(TENSORFLOW_BASE)/TensorFlowLiteCMetal.xcframework/$(PLATFORM)/libTensorFlowLiteCMetal_$(shell echo $(PLATFORM) | cut -d'-' -f1).a

# Libraries to link
# Use -Wl,-force_load for portable kernels to ensure all symbols are included
# (macOS equivalent of -Wl,--whole-archive on Linux)
LIBS = $(TENSORFLOW_LIB) -Wl,-force_load,$(TENSORFLOW_COREML_LIB) -Wl,-force_load,$(TENSORFLOW_METAL_LIB)

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
	@echo "TensorFlowLiteC lib: $(TENSORFLOW_LIB)"
	@if [ -f "$(TENSORFLOW_LIB)" ]; then echo "✅ TensorFlowLiteC library found"; else echo "❌ TensorFlowLiteC library not found"; fi
	@echo "TensorFlowLiteCCoreML lib: $(TENSORFLOW_COREML_LIB)"
	@if [ -f "$(TENSORFLOW_COREML_LIB)" ]; then echo "✅ TensorFlowLiteCCoreML library found"; else echo "❌ TensorFlowLiteCCoreML library not found"; fi
	@echo "TensorFlowLiteCMetal lib: $(TENSORFLOW_METAL_LIB)"
	@if [ -f "$(TENSORFLOW_METAL_LIB)" ]; then echo "✅ TensorFlowLiteCMetal library found"; else echo "❌ TensorFlowLiteCMetal library not found"; fi
	@echo "Headers: $(TENSORFLOW_HEADERS)"
	@if [ -d "$(TENSORFLOW_HEADERS)" ]; then echo "✅ Headers directory found"; else echo "❌ Headers directory not found"; fi
	@echo "Available header directories:"
	@find $(TENSORFLOW_HEADERS) -type d -name "*" | head -10

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
	@echo "Current build mode: $(BUILD_TYPE)"

.PHONY: all debug clean check install help