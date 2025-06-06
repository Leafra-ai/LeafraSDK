#!/bin/bash
# Android build configuration for Executorch with required backends
# This script configures the Android build with XNNPACK, Vulkan, QNN (Qualcomm), and Neuron (MediaTek) backends

set -e

# Set Android configuration environment variables
export ANDROID_NDK="${ANDROID_NDK:-$ANDROID_HOME/ndk-bundle}"
export EXECUTORCH_BUILD_VULKAN=ON
export EXECUTORCH_BUILD_XNNPACK=ON
export EXECUTORCH_BUILD_QNN=ON
export EXECUTORCH_BUILD_NEURON=ON

# Build configuration
echo "Android Executorch Build Configuration:"
echo "- XNNPACK Backend: ENABLED"
echo "- Vulkan Backend: ENABLED"
echo "- QNN (Qualcomm NPU) Backend: ENABLED"
echo "- Neuron (MediaTek NPU) Backend: ENABLED"
echo ""

# Note: This script sets up environment variables for Android build
# To actually build for Android, run:
# ./scripts/build_android_library.sh

echo "Environment configured for Android build with all required backends."
echo "Run ./scripts/build_android_library.sh to build the Android libraries." 