#!/bin/bash

# LeafraSDK Build Script
# This script builds the C++ SDK for various platforms

set -e

# Configuration
BUILD_DIR="build"
INSTALL_DIR="install"
CMAKE_BUILD_TYPE="Release"
VERBOSE=false

# Bundle models 
# Paths are relative to the sdk directory, they are checked and converted to absolute paths before being passed to the cmake build
BUNDLE_MODELS=true 
TOKENIZER_MODEL_PATHS=("corecpp/third_party/models/embedding/multilingual-e5-small/sentencepiece.bpe.model" "corecpp/third_party/models/embedding/multilingual-e5-small/tokenizer_config.json")
EMBEDDING_MODEL_PATHS=("corecpp/third_party/models/embedding/generated_models/coreml/e5_embedding_model_i512a512_FP32.mlmodelc")
LLM_MODEL_PATHS=("corecpp/third_party/models/llm/unsloth/Llama-3.2-3B-Instruct-Q4_K_M.gguf")
# Function to check if file exists and exit if not found
check_file_exists() {
    local path="$1"
    if [ ! -e "$path" ]; then
        echo "ERROR: Required model file/directory not found: $path"
        echo "Please ensure all model files and directories are present in the correct locations"
        exit 1
    fi
}

# If bundling models is enabled, validate all paths exist before merging
if [ "$BUNDLE_MODELS" = true ]; then
    # Initialize empty array for bundled model paths
    BUNDLED_MODEL_PATHS=()
    
    # Validate all tokenizer model paths exist
    for path in "${TOKENIZER_MODEL_PATHS[@]}"; do
        check_file_exists "$path"
    done
    
    # Validate all embedding model paths exist
    for path in "${EMBEDDING_MODEL_PATHS[@]}"; do
        check_file_exists "$path"
    done
    
    # Validate all LLM model paths exist
    for path in "${LLM_MODEL_PATHS[@]}"; do
        check_file_exists "$path"
    done
    
    # If we get here, all paths are valid - merge them and convert to absolute paths
    BUNDLED_MODEL_PATHS=()
    for path in "${TOKENIZER_MODEL_PATHS[@]}" "${EMBEDDING_MODEL_PATHS[@]}" "${LLM_MODEL_PATHS[@]}"; do
        BUNDLED_MODEL_PATHS+=("$(realpath "$path")")
    done
    
    echo "Successfully validated ${#BUNDLED_MODEL_PATHS[@]} model files for bundling (converted to absolute paths)"
fi



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
    echo "  --embedding-fw=FW   Specify embedding framework (coreml, tflite)"
    echo ""
    echo "Targets:"
    echo "  all                 Build all targets (default)"
    echo "  core                Build core library only"
    echo "  tests               Build tests only"
    echo ""
    echo "Examples:"
    echo "  $0                              # Build all targets for current platform"
    echo "  $0 --ios core                   # Build core library for iOS"
    echo "  $0 --macos --embedding-fw=tf    # Build for macOS with TensorFlow"
    echo "  $0 --ios --embedding-fw=coreml  # Build for iOS with CoreML"
    echo "  $0 --clean --debug              # Clean build and build in debug mode"
}

print_info() {
    echo "[INFO] $1"
}

print_error() {
    echo "[ERROR] $1" >&2
}

# Function to run pod install for React Native integration
run_pod_install() {
    local platform=$1
    local react_native_dir="../example/Leafra"
    
    # Ask user if they want to run pod install
    echo ""
    echo "═══════════════════════════════════════════════════════════════════════════════"
    echo "📦 COCOAPODS INTEGRATION"
    echo "═══════════════════════════════════════════════════════════════════════════════"
    echo "The build is complete, but you may want to update CocoaPods dependencies"
    echo "to ensure the React Native app uses the newly built $platform frameworks."
    echo ""
    echo "⚠️  Note: This step is not always required and can be skipped if:"
    echo "   • You're not working with the React Native example app"
    echo "   • You've already run pod install recently"
    echo "   • You only need the native SDK libraries"
    echo ""
    read -p "Do you want to run 'pod install' for React Native integration? (Y/n): " -n 1 -r
    echo ""
    
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        print_info "⏭️  Skipping pod install. You can run it manually later if needed:"
        print_info "   cd example/Leafra/ios && pod install"
        echo "═══════════════════════════════════════════════════════════════════════════════"
        return 0
    fi
    
    print_info "📦 Running pod install for React Native integration ($platform)..."
    echo "═══════════════════════════════════════════════════════════════════════════════"
    echo "🔄 UPDATING COCOAPODS DEPENDENCIES - This ensures the updated frameworks are linked"
    echo "═══════════════════════════════════════════════════════════════════════════════"
    
    if [ -d "$react_native_dir" ]; then
        cd "$react_native_dir"
        if [ -f "ios/Podfile" ]; then
            cd ios
            if command -v pod >/dev/null 2>&1; then
                print_info "  → Running 'pod install' in $react_native_dir/ios/"
                pod install --verbose || {
                    print_error "pod install failed! You may need to run it manually."
                    cd ../../..
                    return 1
                }
                print_info "  ✅ CocoaPods integration completed successfully!"
                echo "🚀 React Native app is now ready with updated $platform frameworks!"
            else
                print_error "CocoaPods not found! Please install with: sudo gem install cocoapods"
                cd ../../..
                return 1
            fi
            cd ../..
        else
            print_error "Podfile not found in $react_native_dir/ios/"
            cd ../..
            return 1
        fi
        cd ..
    else
        print_error "React Native directory not found: $react_native_dir"
        return 1
    fi
    
    echo "═══════════════════════════════════════════════════════════════════════════════"
    echo "📱 IMPORTANT: If you have Xcode open, please close and reopen the workspace"
    echo "   to ensure all framework changes are properly loaded:"
    echo "   → open example/Leafra/ios/DokuChat.xcworkspace"
    echo "═══════════════════════════════════════════════════════════════════════════════"
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

# Copy frameworks/libraries to React Native package structure for distribution
copy_frameworks_to_react_native() {
    local platform="$1"
    local simulator="${2:-false}"
    
    print_info "📦 Copying frameworks/libraries to React Native package structure..."
    
    # Define source and destination paths
    local src_dir="$INSTALL_DIR"
    local rn_dir="react-native"
    
    # Platform-specific paths and file extensions
    case "$platform" in
        "ios")
            if [ "$simulator" = true ]; then
                local src_platform_dir="$src_dir/ios-simulator"
                local dest_platform_dir="$rn_dir/ios"
            else
                local src_platform_dir="$src_dir/ios"
                local dest_platform_dir="$rn_dir/ios"
            fi
            local framework_ext=".framework"
            local lib_subdir="Frameworks"
            ;;
        "macos")
            local src_platform_dir="$src_dir/macos"
            local dest_platform_dir="$rn_dir/macos"
            local framework_ext=".framework"
            local lib_subdir="Frameworks"
            ;;
        "android")
            local src_platform_dir="$src_dir/android"
            local dest_platform_dir="$rn_dir/android"
            local framework_ext=".so"
            local lib_subdir="lib"
            ;;
        "windows")
            local src_platform_dir="$src_dir/windows"
            local dest_platform_dir="$rn_dir/windows"
            local framework_ext=".dll"
            local lib_subdir="bin"
            ;;
        "linux")
            local src_platform_dir="$src_dir/linux"
            local dest_platform_dir="$rn_dir/linux"
            local framework_ext=".so"
            local lib_subdir="lib"
            ;;
        *)
            print_error "Unsupported platform for React Native framework copying: $platform"
            return 1
            ;;
    esac
    
    # Check if source libraries exist
    if [ ! -d "$src_platform_dir/$lib_subdir" ]; then
        print_error "Source libraries directory not found: $src_platform_dir/$lib_subdir"
        print_error "Make sure the build completed successfully before copying frameworks."
        return 1
    fi
    
    # Create destination directory
    mkdir -p "$dest_platform_dir"
    
    # Platform-specific copying logic
    case "$platform" in
        "ios"|"macos")
            # Copy frameworks for Apple platforms
            if [ -d "$src_platform_dir/$lib_subdir/LeafraCore$framework_ext" ]; then
                print_info "  → Copying LeafraCore$framework_ext to $dest_platform_dir/"
                cp -R "$src_platform_dir/$lib_subdir/LeafraCore$framework_ext" "$dest_platform_dir/"
                print_info "    ✅ LeafraCore$framework_ext copied successfully"
                
                # Copy corresponding dSYM files with proper App Store naming
                local framework_dsym_name="LeafraCore.framework.dSYM"
                if [ -d "$src_platform_dir/$lib_subdir/$framework_dsym_name" ]; then
                    print_info "  → Copying $framework_dsym_name to $dest_platform_dir/"
                    cp -R "$src_platform_dir/$lib_subdir/$framework_dsym_name" "$dest_platform_dir/"
                    print_info "    ✅ $framework_dsym_name copied successfully"
                else
                    print_info "  ⚠️  $framework_dsym_name not found at $src_platform_dir/$lib_subdir/"
                    
                    # Check if it might be in the parent directory
                    if [ -d "$src_platform_dir/$framework_dsym_name" ]; then
                        print_info "  → Found $framework_dsym_name in parent directory, copying..."
                        cp -R "$src_platform_dir/$framework_dsym_name" "$dest_platform_dir/"
                        print_info "    ✅ $framework_dsym_name copied from parent directory"
                    else
                        print_info "  ⚠️  $framework_dsym_name not found in parent directory either"
                    fi
                fi
            else
                print_error "LeafraCore$framework_ext not found in $src_platform_dir/$lib_subdir/"
                return 1
            fi
            
            if [ -d "$src_platform_dir/$lib_subdir/llama$framework_ext" ]; then
                print_info "  → Copying llama$framework_ext to $dest_platform_dir/"
                cp -R "$src_platform_dir/$lib_subdir/llama$framework_ext" "$dest_platform_dir/"
                print_info "    ✅ llama$framework_ext copied successfully"
                
                # Copy and rename llama dSYM files for App Store compliance
                local llama_framework_dsym_name="llama.framework.dSYM"
                if [ -d "$src_platform_dir/$lib_subdir/llama.dSYM" ]; then
                    print_info "  → Copying and renaming llama.dSYM to $llama_framework_dsym_name"
                    cp -R "$src_platform_dir/$lib_subdir/llama.dSYM" "$dest_platform_dir/$llama_framework_dsym_name"
                    print_info "    ✅ $llama_framework_dsym_name copied successfully"
                elif [ -d "$src_platform_dir/$lib_subdir/$llama_framework_dsym_name" ]; then
                    print_info "  → Copying $llama_framework_dsym_name to $dest_platform_dir/"
                    cp -R "$src_platform_dir/$lib_subdir/$llama_framework_dsym_name" "$dest_platform_dir/"
                    print_info "    ✅ $llama_framework_dsym_name copied successfully"
                else
                    print_info "  ⚠️  llama dSYM not found (llama.framework comes from prebuilt XCFramework)"
                fi
            else
                print_error "llama$framework_ext not found in $src_platform_dir/$lib_subdir/"
                return 1
            fi
            ;;
        "android"|"windows"|"linux")
            # Copy shared libraries for other platforms
            # Create lib subdirectory in destination
            mkdir -p "$dest_platform_dir/$lib_subdir"
            
            # Define expected library names based on platform
            case "$platform" in
                "android"|"linux")
                    local leafra_lib="libLeafraCore$framework_ext"
                    local llama_lib="libllama$framework_ext"
                    ;;
                "windows")
                    local leafra_lib="LeafraCore$framework_ext"
                    local llama_lib="llama$framework_ext"
                    ;;
            esac
            
            if [ -f "$src_platform_dir/$lib_subdir/$leafra_lib" ]; then
                print_info "  → Copying $leafra_lib to $dest_platform_dir/$lib_subdir/"
                cp "$src_platform_dir/$lib_subdir/$leafra_lib" "$dest_platform_dir/$lib_subdir/"
                print_info "    ✅ $leafra_lib copied successfully"
            else
                print_error "$leafra_lib not found in $src_platform_dir/$lib_subdir/"
                return 1
            fi
            
            if [ -f "$src_platform_dir/$lib_subdir/$llama_lib" ]; then
                print_info "  → Copying $llama_lib to $dest_platform_dir/$lib_subdir/"
                cp "$src_platform_dir/$lib_subdir/$llama_lib" "$dest_platform_dir/$lib_subdir/"
                print_info "    ✅ $llama_lib copied successfully"
            else
                print_error "$llama_lib not found in $src_platform_dir/$lib_subdir/"
                return 1
            fi
            
            # Copy any additional runtime dependencies if they exist
            if [ "$platform" = "windows" ]; then
                # Copy any .pdb files for debugging (optional)
                if [ -f "$src_platform_dir/$lib_subdir/LeafraCore.pdb" ]; then
                    print_info "  → Copying LeafraCore.pdb (debug symbols) to $dest_platform_dir/$lib_subdir/"
                    cp "$src_platform_dir/$lib_subdir/LeafraCore.pdb" "$dest_platform_dir/$lib_subdir/"
                    print_info "    ✅ LeafraCore.pdb copied successfully"
                fi
                if [ -f "$src_platform_dir/$lib_subdir/llama.pdb" ]; then
                    print_info "  → Copying llama.pdb (debug symbols) to $dest_platform_dir/$lib_subdir/"
                    cp "$src_platform_dir/$lib_subdir/llama.pdb" "$dest_platform_dir/$lib_subdir/"
                    print_info "    ✅ llama.pdb copied successfully"
                fi
            fi
            ;;
    esac
    
    print_info "📦 Framework/library copying completed for $platform"
    print_info "    Libraries are now available in: $dest_platform_dir/"
    print_info "    The React Native package can now be used with these libraries."
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
        local install_dir="ios-simulator"
    else
        local sdk="iphoneos"
        local arch="arm64"
        local build_dir="${BUILD_DIR}/ios"
        local platform="ios"
        local install_dir="ios"
    fi
    
    # Check and build dependencies
    check_and_build_dependencies "$platform"
    
    mkdir -p "$build_dir"
    cd "$build_dir"
    
    cmake ../.. \
        -DCMAKE_SYSTEM_NAME=iOS \
        -DCMAKE_OSX_SYSROOT="$sdk" \
        -DCMAKE_OSX_ARCHITECTURES="$arch" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET="16.4" \
        -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
        -DCMAKE_INSTALL_PREFIX="../../$INSTALL_DIR/$install_dir" \
        -DLEAFRA_BUILD_RN_BINDINGS=ON \
        -DCMAKE_STRIP="" \
        -DCMAKE_C_FLAGS_RELEASE="-O2 -g -DNDEBUG" \
        -DCMAKE_CXX_FLAGS_RELEASE="-O2 -g -DNDEBUG" \
        ${EMBEDDING_FW:+-DLEAFRA_EMBEDDING_FRAMEWORK="$EMBEDDING_FW"} \
        ${BUNDLED_MODEL_PATHS:+-DLEAFRA_BUNDLED_MODEL_PATHS="$(IFS=';'; echo "${BUNDLED_MODEL_PATHS[*]}")"} \
        ${VERBOSE:+-DCMAKE_VERBOSE_MAKEFILE=ON}
    
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu) $target
    make install
    
    cd ../..
    
    # Copy frameworks to React Native package after successful build
    print_info "🚀 Build completed successfully for iOS!"
    copy_frameworks_to_react_native "ios" "$simulator"
    if [ "$simulator" = false ]; then
        run_pod_install "ios"
    fi
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
        -DCMAKE_STRIP="" \
        -DCMAKE_C_FLAGS_RELEASE="-O2 -g -DNDEBUG" \
        -DCMAKE_CXX_FLAGS_RELEASE="-O2 -g -DNDEBUG" \
        ${EMBEDDING_FW:+-DLEAFRA_EMBEDDING_FRAMEWORK="$EMBEDDING_FW"} \
        ${BUNDLED_MODEL_PATHS:+-DLEAFRA_BUNDLED_MODEL_PATHS="$(IFS=';'; echo "${BUNDLED_MODEL_PATHS[*]}")"} \
        ${VERBOSE:+-DCMAKE_VERBOSE_MAKEFILE=ON}
    
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu) $target
    make install
    
    cd ../..
    
    # Copy frameworks to React Native package after successful build
    print_info "🚀 Build completed successfully for macOS!"
    copy_frameworks_to_react_native "macos"
    run_pod_install "macos"
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
        ${EMBEDDING_FW:+-DLEAFRA_EMBEDDING_FRAMEWORK="$EMBEDDING_FW"} \
        ${VERBOSE:+-DCMAKE_VERBOSE_MAKEFILE=ON}
    
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu) $target
    make install
    
    cd ../..
    
    # Copy libraries to React Native package after successful build
    print_info "🚀 Build completed successfully for Android!"
    copy_frameworks_to_react_native "android"
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
        ${EMBEDDING_FW:+-DLEAFRA_EMBEDDING_FRAMEWORK="$EMBEDDING_FW"} \
        ${VERBOSE:+-DCMAKE_VERBOSE_MAKEFILE=ON}
    
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu) $target
    make install
    
    cd ../..
    
    # Copy libraries to React Native package after successful build (for supported platforms)
    case "$PLATFORM" in
        "windows"|"linux")
            print_info "🚀 Build completed successfully for $PLATFORM!"
            copy_frameworks_to_react_native "$PLATFORM"
            ;;
        *)
            print_info "🚀 Build completed successfully for $PLATFORM!"
            print_info "Note: React Native package copying not yet implemented for $PLATFORM"
            ;;
    esac
}

# Parse arguments
CLEAN=false
IOS=false
MACOS=false
ANDROID=false
SIMULATOR=false
TARGET="all"
EMBEDDING_FW=""

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
        --embedding-fw=*)
            EMBEDDING_FW="${1#*=}"
            if [[ "$EMBEDDING_FW" != "tflite" && "$EMBEDDING_FW" != "tf" && "$EMBEDDING_FW" != "coreml" ]]; then
                print_error "Invalid embedding framework: $EMBEDDING_FW"
                print_error ""
                print_error "Valid options by platform:"
                print_error "  tflite  - TensorFlow Lite (iOS/Android only) [NOT IMPLEMENTED]"
                print_error "  tf      - TensorFlow (macOS/Windows only) [NOT IMPLEMENTED]" 
                print_error "  coreml  - CoreML (iOS/macOS only)"
                exit 1
            fi
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

# Check if embedding framework is specified (mandatory)
if [ -z "$EMBEDDING_FW" ]; then
    print_error "Embedding framework is required. Use --embedding-fw=<framework>"
    print_error ""
    print_error "Valid options by platform:"
    print_error "  tflite  - TensorFlow Lite (iOS/Android only) [NOT IMPLEMENTED]"
    print_error "  tf      - TensorFlow (macOS/Windows only) [NOT IMPLEMENTED]" 
    print_error "  coreml  - CoreML (iOS/macOS only)"
    print_error ""
    print_usage
    exit 1
fi

print_info "Embedding Framework: $EMBEDDING_FW"

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