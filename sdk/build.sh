#!/bin/bash

# LeafraSDK Build Script
# This script builds the C++ SDK for various platforms

set -e

# Configuration
BUILD_DIR="build"
INSTALL_DIR="install"
CMAKE_BUILD_TYPE="Release"
VERBOSE=false

# Platform detection
if [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="apple"
    if [[ $(uname -m) == "arm64" ]]; then
        ARCH="arm64"
    else
        ARCH="x86_64"
    fi
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="linux"
    ARCH=$(uname -m)
elif [[ "$OSTYPE" == "msys" ]]; then
    PLATFORM="windows"
    ARCH="x86_64"
else
    echo "Unsupported platform: $OSTYPE"
    exit 1
fi

# Functions
print_usage() {
    echo "Usage: $0 [OPTIONS] [TARGETS]"
    echo ""
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -v, --verbose       Enable verbose output"
    echo "  -d, --debug         Build in debug mode"
    echo "  -c, --clean         Clean build directory before building"
    echo "  --ios               Build for iOS"
    echo "  --macos             Build for macOS"
    echo "  --android           Build for Android"
    echo "  --simulator         Build for iOS Simulator (when using --ios)"
    echo ""
    echo "Targets:"
    echo "  core                Build core C++ library only"
    echo "  bindings            Build React Native bindings"
    echo "  all                 Build everything (default)"
    echo ""
    echo "Examples:"
    echo "  $0                  # Build everything for current platform"
    echo "  $0 --ios core       # Build core library for iOS"
    echo "  $0 --macos bindings # Build RN bindings for macOS"
    echo "  $0 --clean --debug  # Clean build and build in debug mode"
}

print_info() {
    echo "[INFO] $1"
}

print_error() {
    echo "[ERROR] $1" >&2
}

check_and_build_dependencies() {
    local platform=$1
    print_info "Checking dependencies for $platform..."
    
    # Check SentencePiece
    if [ "$platform" = "ios" ]; then
        local sp_lib="corecpp/third_party/prebuilt/sentencepiece/ios-device/lib/libsentencepiece.a"
        local makefile_target="ios"
    elif [ "$platform" = "ios-simulator" ]; then
        local sp_lib="corecpp/third_party/prebuilt/sentencepiece/ios-simulator/lib/libsentencepiece.a"
        local makefile_target="ios"
    elif [ "$platform" = "macos" ]; then
        local sp_lib="corecpp/third_party/prebuilt/sentencepiece/macos/lib/libsentencepiece.a"
        local makefile_target="macos"
    fi
    
    if [ ! -f "$sp_lib" ]; then
        print_info "SentencePiece not found for $platform, building..."
        cd corecpp/third_party
        make -f Makefile.sentencepiece "$makefile_target" || { print_error "Failed to build SentencePiece for $platform"; exit 1; }
        cd ../..
        print_info "✅ SentencePiece built successfully for $platform"
    else
        print_info "✅ SentencePiece found for $platform"
    fi
}

clean_sentencepiece_artifacts() {
    local platform=${1:-"all"}
    local sp_dir="corecpp/third_party/prebuilt/sentencepiece"
    
    if [ ! -d "$sp_dir" ]; then
        return 0
    fi
    
    case $platform in
        "all")
            print_info "Cleaning all SentencePiece artifacts..."
            rm -rf "$sp_dir"/*
            ;;
        "ios")
            print_info "Cleaning iOS device SentencePiece artifacts..."
            rm -rf "$sp_dir"/ios-device "$sp_dir"/build/ios
            ;;
        "ios-simulator")
            print_info "Cleaning iOS simulator SentencePiece artifacts..."
            rm -rf "$sp_dir"/ios-simulator* "$sp_dir"/build/ios
            ;;
        "macos")
            print_info "Cleaning macOS SentencePiece artifacts..."
            rm -rf "$sp_dir"/macos "$sp_dir"/build/macos
            ;;
    esac
}

clean_sentencepiece_intermediates() {
    local sp_dir="corecpp/third_party/prebuilt/sentencepiece"
    
    if [ -d "$sp_dir" ]; then
        print_info "Cleaning SentencePiece intermediate files..."
        # Keep only final artifacts: ios-device, ios-simulator-universal, sentencepiece.xcframework
        rm -rf "$sp_dir"/build
        rm -rf "$sp_dir"/ios-simulator-x86_64
        rm -rf "$sp_dir"/ios-simulator-arm64
        rm -rf "$sp_dir"/ios-simulator
        # Keep ios-device, ios-simulator-universal, sentencepiece.xcframework
    fi
}

clean_build() {
    local platform=${1:-"all"}
    
    if [ -d "$BUILD_DIR" ]; then
        print_info "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
    fi
    if [ -d "$INSTALL_DIR" ]; then
        print_info "Cleaning install directory..."
        rm -rf "$INSTALL_DIR"
    fi
    
    # Clean SentencePiece artifacts based on platform
    clean_sentencepiece_artifacts "$platform"
}

build_ios() {
    local target=${1:-"all"}
    local simulator=${2:-false}
    
    print_info "Building for iOS (target: $target, simulator: $simulator)..."
    
    if [ "$simulator" = true ]; then
        local sdk="iphonesimulator"
        local arch="x86_64;arm64"
        local build_dir="${BUILD_DIR}/ios-simulator"
        local platform="ios-simulator"
    else
        local sdk="iphoneos"
        local arch="arm64"
        local build_dir="${BUILD_DIR}/ios"
        local platform="ios"
    fi
    
    # Check and build dependencies
    check_and_build_dependencies "$platform"
    
    mkdir -p "$build_dir"
    cd "$build_dir"
    
    cmake ../.. \
        -DCMAKE_SYSTEM_NAME=iOS \
        -DCMAKE_OSX_SYSROOT="$sdk" \
        -DCMAKE_OSX_ARCHITECTURES="$arch" \
        -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
        -DCMAKE_INSTALL_PREFIX="../../$INSTALL_DIR/ios" \
        -DLEAFRA_BUILD_RN_BINDINGS=OFF \
        ${VERBOSE:+-DCMAKE_VERBOSE_MAKEFILE=ON}
    
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu) $target
    make install
    
    cd ../..
}

build_macos() {
    local target=${1:-"all"}
    
    print_info "Building for macOS (target: $target)..."
    
    # Check and build dependencies
    check_and_build_dependencies "macos"
    
    local build_dir="${BUILD_DIR}/macos"
    mkdir -p "$build_dir"
    cd "$build_dir"
    
    cmake ../.. \
        -DCMAKE_SYSTEM_NAME=Darwin \
        -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
        -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
        -DCMAKE_INSTALL_PREFIX="../../$INSTALL_DIR/macos" \
        -DLEAFRA_BUILD_RN_BINDINGS=ON \
        ${VERBOSE:+-DCMAKE_VERBOSE_MAKEFILE=ON}
    
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu) $target
    make install
    
    cd ../..
}

build_android() {
    local target=${1:-"all"}
    local ndk_path="${ANDROID_NDK:-$ANDROID_NDK_HOME}"
    
    if [ -z "$ndk_path" ]; then
        print_error "Android NDK not found. Set ANDROID_NDK or ANDROID_NDK_HOME environment variable."
        exit 1
    fi
    
    print_info "Building for Android (target: $target)..."
    print_info "Using NDK: $ndk_path"
    
    local build_dir="${BUILD_DIR}/android"
    mkdir -p "$build_dir"
    cd "$build_dir"
    
    cmake ../.. \
        -DCMAKE_TOOLCHAIN_FILE="$ndk_path/build/cmake/android.toolchain.cmake" \
        -DANDROID_ABI=arm64-v8a \
        -DANDROID_NATIVE_API_LEVEL=23 \
        -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
        -DCMAKE_INSTALL_PREFIX="../../$INSTALL_DIR/android" \
        -DLEAFRA_BUILD_RN_BINDINGS=ON \
        ${VERBOSE:+-DCMAKE_VERBOSE_MAKEFILE=ON}
    
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu) $target
    make install
    
    cd ../..
}

build_current_platform() {
    local target=${1:-"all"}
    
    print_info "Building for current platform: $PLATFORM-$ARCH (target: $target)..."
    
    local build_dir="${BUILD_DIR}/$PLATFORM"
    mkdir -p "$build_dir"
    cd "$build_dir"
    
    cmake ../.. \
        -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
        -DCMAKE_INSTALL_PREFIX="../../$INSTALL_DIR/$PLATFORM" \
        -DLEAFRA_BUILD_RN_BINDINGS=ON \
        ${VERBOSE:+-DCMAKE_VERBOSE_MAKEFILE=ON}
    
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu) $target
    make install
    
    cd ../..
}

# Parse arguments
CLEAN=false
IOS=false
MACOS=false
ANDROID=false
SIMULATOR=false
TARGET="all"

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            print_usage
            exit 0
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -d|--debug)
            CMAKE_BUILD_TYPE="Debug"
            shift
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        --ios)
            IOS=true
            shift
            ;;
        --macos)
            MACOS=true
            shift
            ;;
        --android)
            ANDROID=true
            shift
            ;;
        --simulator)
            SIMULATOR=true
            shift
            ;;
        core|bindings|all)
            TARGET="$1"
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            print_usage
            exit 1
            ;;
    esac
done

# Main execution
print_info "LeafraSDK Build Script"
print_info "Platform: $PLATFORM-$ARCH"
print_info "Build Type: $CMAKE_BUILD_TYPE"
print_info "Target: $TARGET"

if [ "$CLEAN" = true ]; then
    # Determine platform for cleaning
    if [ "$IOS" = true ]; then
        if [ "$SIMULATOR" = true ]; then
            clean_build "ios-simulator"
        else
            clean_build "ios"
        fi
    elif [ "$MACOS" = true ]; then
        clean_build "macos"
    elif [ "$ANDROID" = true ]; then
        clean_build "android"
    else
        clean_build "all"
    fi
fi

# Create necessary directories
mkdir -p "$BUILD_DIR"
mkdir -p "$INSTALL_DIR"

# Build for specified platforms
if [ "$IOS" = true ]; then
    build_ios "$TARGET" "$SIMULATOR"
    # Clean up SentencePiece intermediates after successful build
    if [ "$SIMULATOR" = true ]; then
        clean_sentencepiece_intermediates
    fi
elif [ "$MACOS" = true ]; then
    build_macos "$TARGET"
elif [ "$ANDROID" = true ]; then
    build_android "$TARGET"
else
    # Build for current platform
    build_current_platform "$TARGET"
fi

print_info "Build completed successfully!"
print_info "Install directory: $INSTALL_DIR" 