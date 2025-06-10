#!/usr/bin/env python3
"""
SentencePiece Cross-Platform Build Script
Builds SentencePiece from source for iOS/macOS/Android/Windows/Linux
with libgoogle-perftools-dev support where applicable.
"""

import os
import sys
import subprocess
import platform
import shutil
import argparse
from pathlib import Path

class SentencePieceBuildError(Exception):
    pass

class SentencePieceBuilder:
    def __init__(self, source_dir, output_dir, target_platform=None):
        self.source_dir = Path(source_dir).resolve()
        self.output_dir = Path(output_dir).resolve()
        self.target_platform = target_platform or self.detect_platform()
        self.build_dir = self.output_dir / "build" / self.target_platform
        
        # Ensure directories exist
        self.output_dir.mkdir(parents=True, exist_ok=True)
        self.build_dir.mkdir(parents=True, exist_ok=True)
        
        print(f"Building SentencePiece for {self.target_platform}")
        print(f"Source: {self.source_dir}")
        print(f"Output: {self.output_dir}")
        print(f"Build:  {self.build_dir}")

    def detect_platform(self):
        """Detect the current platform"""
        system = platform.system().lower()
        if system == "darwin":
            return "macos"
        elif system == "linux":
            return "linux"
        elif system == "windows":
            return "windows"
        else:
            raise SentencePieceBuildError(f"Unsupported platform: {system}")

    def run_command(self, cmd, cwd=None, env=None):
        """Run a command and handle errors"""
        print(f"Running: {' '.join(cmd)}")
        try:
            result = subprocess.run(
                cmd, 
                cwd=cwd or self.build_dir, 
                env=env,
                check=True,
                capture_output=True,
                text=True
            )
            if result.stdout:
                print(result.stdout)
            return result
        except subprocess.CalledProcessError as e:
            print(f"Command failed: {e}")
            if e.stdout:
                print(f"STDOUT: {e.stdout}")
            if e.stderr:
                print(f"STDERR: {e.stderr}")
            raise SentencePieceBuildError(f"Build command failed: {' '.join(cmd)}")

    def install_dependencies_linux(self):
        """Install dependencies on Linux"""
        print("Installing Linux dependencies...")
        
        # Check if we can use apt-get
        try:
            self.run_command([
                "sudo", "apt-get", "update"
            ])
            self.run_command([
                "sudo", "apt-get", "install", "-y",
                "cmake", "build-essential", "pkg-config", 
                "libgoogle-perftools-dev", "libtcmalloc-minimal4"
            ])
        except:
            print("Warning: Could not install dependencies via apt-get")
            print("Please ensure the following are installed:")
            print("- cmake")
            print("- build-essential") 
            print("- pkg-config")
            print("- libgoogle-perftools-dev")
            print("- libtcmalloc-minimal4")

    def install_dependencies_macos(self):
        """Install dependencies on macOS"""
        print("Installing macOS dependencies...")
        
        # Check if Homebrew is available
        try:
            self.run_command(["brew", "--version"])
            self.run_command([
                "brew", "install", "cmake", "gperftools"
            ])
        except:
            print("Warning: Could not install dependencies via Homebrew")
            print("Please ensure the following are installed:")
            print("- cmake (via Homebrew: brew install cmake)")
            print("- gperftools (via Homebrew: brew install gperftools)")

    def install_dependencies_windows(self):
        """Install dependencies on Windows"""
        print("Installing Windows dependencies...")
        print("Please ensure the following are installed:")
        print("- Visual Studio 2019 or later with C++ tools")
        print("- CMake")
        print("- vcpkg (optional, for gperftools)")

    def get_cmake_args_common(self):
        """Get common CMake arguments"""
        return [
            f"-DCMAKE_INSTALL_PREFIX={self.output_dir / self.target_platform}",
            "-DCMAKE_BUILD_TYPE=Release",
            "-DSPM_ENABLE_SHARED=ON",
            "-DSPM_BUILD_TEST=OFF",
            "-DSPM_ENABLE_TCMALLOC=ON",
            "-DSPM_TCMALLOC_STATIC=OFF",
            "-DSPM_PROTOBUF_PROVIDER=internal",
            "-DSPM_ABSL_PROVIDER=internal",
            "-DCMAKE_CXX_STANDARD=17",
        ]

    def build_linux(self):
        """Build for Linux"""
        self.install_dependencies_linux()
        
        cmake_args = self.get_cmake_args_common() + [
            "-DCMAKE_C_COMPILER=gcc",
            "-DCMAKE_CXX_COMPILER=g++",
            "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
        ]
        
        # Configure
        self.run_command(["cmake"] + cmake_args + [str(self.source_dir)])
        
        # Build
        self.run_command(["make", "-j", str(os.cpu_count() or 4)])
        
        # Install
        self.run_command(["make", "install"])

    def build_macos(self):
        """Build for macOS"""
        self.install_dependencies_macos()
        
        cmake_args = self.get_cmake_args_common() + [
            "-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15",
            "-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64",
            "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
        ]
        
        # Configure
        self.run_command(["cmake"] + cmake_args + [str(self.source_dir)])
        
        # Build
        self.run_command(["make", "-j", str(os.cpu_count() or 4)])
        
        # Install
        self.run_command(["make", "install"])

    def build_ios(self):
        """Build for iOS"""
        print("Building for iOS...")
        
        # iOS Device (arm64)
        ios_device_dir = self.build_dir / "ios-device"
        ios_device_dir.mkdir(parents=True, exist_ok=True)
        
        # Use SentencePiece's iOS toolchain
        ios_toolchain = self.source_dir / "cmake" / "ios.toolchain.cmake"
        
        cmake_args_device = self.get_cmake_args_common() + [
            f"-DCMAKE_INSTALL_PREFIX={self.output_dir / 'ios-device'}",
            f"-DCMAKE_TOOLCHAIN_FILE={ios_toolchain}",
            "-DPLATFORM=OS64",  # iOS device
            "-DDEPLOYMENT_TARGET=15.1",
            "-DSPM_ENABLE_SHARED=OFF",  # iOS requires static libraries
            "-DSPM_ENABLE_TCMALLOC=OFF",  # TCMalloc not available on iOS
            "-DSPM_BUILD_EXECUTABLES=OFF",  # Disable executables to avoid set_xcode_property issues
        ]
        
        self.run_command(["cmake"] + cmake_args_device + [str(self.source_dir)], cwd=ios_device_dir)
        self.run_command(["make", "-j", str(os.cpu_count() or 4)], cwd=ios_device_dir)
        self.run_command(["make", "install"], cwd=ios_device_dir)
        
        # iOS Simulator x86_64
        ios_sim_x86_dir = self.build_dir / "ios-simulator-x86_64"
        ios_sim_x86_dir.mkdir(parents=True, exist_ok=True)
        
        cmake_args_sim_x86 = self.get_cmake_args_common() + [
            f"-DCMAKE_INSTALL_PREFIX={self.output_dir / 'ios-simulator-x86_64'}",
            f"-DCMAKE_TOOLCHAIN_FILE={ios_toolchain}",
            "-DPLATFORM=SIMULATOR64",  # iOS simulator x86_64
            "-DDEPLOYMENT_TARGET=15.1",
            "-DSPM_ENABLE_SHARED=OFF",
            "-DSPM_ENABLE_TCMALLOC=OFF",
            "-DSPM_BUILD_EXECUTABLES=OFF",
        ]
        
        self.run_command(["cmake"] + cmake_args_sim_x86 + [str(self.source_dir)], cwd=ios_sim_x86_dir)
        self.run_command(["make", "-j", str(os.cpu_count() or 4)], cwd=ios_sim_x86_dir)
        self.run_command(["make", "install"], cwd=ios_sim_x86_dir)
        
        # iOS Simulator ARM64
        ios_sim_arm64_dir = self.build_dir / "ios-simulator-arm64"
        ios_sim_arm64_dir.mkdir(parents=True, exist_ok=True)
        
        cmake_args_sim_arm64 = self.get_cmake_args_common() + [
            f"-DCMAKE_INSTALL_PREFIX={self.output_dir / 'ios-simulator-arm64'}",
            f"-DCMAKE_TOOLCHAIN_FILE={ios_toolchain}",
            "-DPLATFORM=SIMULATORARM64",  # iOS simulator ARM64
            "-DDEPLOYMENT_TARGET=15.1",
            "-DSPM_ENABLE_SHARED=OFF",
            "-DSPM_ENABLE_TCMALLOC=OFF",
            "-DSPM_BUILD_EXECUTABLES=OFF",
        ]
        
        self.run_command(["cmake"] + cmake_args_sim_arm64 + [str(self.source_dir)], cwd=ios_sim_arm64_dir)
        self.run_command(["make", "-j", str(os.cpu_count() or 4)], cwd=ios_sim_arm64_dir)
        self.run_command(["make", "install"], cwd=ios_sim_arm64_dir)
        
        # Create universal iOS simulator libraries
        self.create_universal_simulator_libs()
        
        # Create XCFramework
        self.create_xcframework()

    def create_universal_simulator_libs(self):
        """Create universal iOS simulator libraries (x86_64 + ARM64)"""
        print("Creating universal iOS simulator libraries...")
        
        # Create universal output directory
        universal_dir = self.output_dir / "ios-simulator-universal"
        universal_lib_dir = universal_dir / "lib"
        universal_include_dir = universal_dir / "include"
        
        universal_lib_dir.mkdir(parents=True, exist_ok=True)
        universal_include_dir.mkdir(parents=True, exist_ok=True)
        
        # Copy headers from x86_64 build (they should be identical)
        x86_include_dir = self.output_dir / "ios-simulator-x86_64" / "include"
        if x86_include_dir.exists():
            shutil.copytree(x86_include_dir, universal_include_dir, dirs_exist_ok=True)
        
        # Create universal binaries for main library and training library
        libs_to_combine = ["libsentencepiece.a", "libsentencepiece_train.a"]
        
        for lib_name in libs_to_combine:
            x86_lib = self.output_dir / "ios-simulator-x86_64" / "lib" / lib_name
            arm64_lib = self.output_dir / "ios-simulator-arm64" / "lib" / lib_name
            universal_lib = universal_lib_dir / lib_name
            
            if x86_lib.exists() and arm64_lib.exists():
                print(f"Creating universal {lib_name}...")
                self.run_command([
                    "lipo", "-create", 
                    str(x86_lib), str(arm64_lib),
                    "-output", str(universal_lib)
                ])
            elif x86_lib.exists():
                print(f"Only x86_64 {lib_name} found, copying...")
                shutil.copy2(x86_lib, universal_lib)
            elif arm64_lib.exists():
                print(f"Only ARM64 {lib_name} found, copying...")
                shutil.copy2(arm64_lib, universal_lib)
            else:
                print(f"Warning: {lib_name} not found in either architecture")
        
        print("✅ Universal iOS simulator libraries created")
        
        # Clean up intermediate architecture-specific builds
        print("Cleaning up intermediate iOS simulator builds...")
        x86_64_dir = self.output_dir / "ios-simulator-x86_64"
        arm64_dir = self.output_dir / "ios-simulator-arm64"
        
        if x86_64_dir.exists():
            shutil.rmtree(x86_64_dir)
            print("  ✅ Removed ios-simulator-x86_64")
        
        if arm64_dir.exists():
            shutil.rmtree(arm64_dir)
            print("  ✅ Removed ios-simulator-arm64")
        
        print("✅ iOS simulator intermediate files cleaned")

    def create_xcframework(self):
        """Create XCFramework for iOS"""
        print("Creating XCFramework...")
        
        device_lib = self.output_dir / "ios-device" / "lib" / "libsentencepiece.a"
        sim_lib = self.output_dir / "ios-simulator-universal" / "lib" / "libsentencepiece.a"
        xcframework_path = self.output_dir / "sentencepiece.xcframework"
        
        if xcframework_path.exists():
            shutil.rmtree(xcframework_path)
        
        self.run_command([
            "xcodebuild", "-create-xcframework",
            "-library", str(device_lib),
            "-headers", str(self.output_dir / "ios-device" / "include"),
            "-library", str(sim_lib), 
            "-headers", str(self.output_dir / "ios-simulator-universal" / "include"),
            "-output", str(xcframework_path)
        ])

    def build_android(self):
        """Build for Android"""
        print("Building for Android...")
        
        # Check for Android NDK
        ndk_path = os.environ.get("ANDROID_NDK_ROOT") or os.environ.get("NDK_ROOT")
        if not ndk_path:
            raise SentencePieceBuildError("ANDROID_NDK_ROOT environment variable not set")
        
        ndk_path = Path(ndk_path)
        if not ndk_path.exists():
            raise SentencePieceBuildError(f"Android NDK not found at: {ndk_path}")
        
        toolchain_file = ndk_path / "build" / "cmake" / "android.toolchain.cmake"
        if not toolchain_file.exists():
            raise SentencePieceBuildError(f"Android toolchain not found at: {toolchain_file}")
        
        # Build for multiple ABIs
        abis = ["arm64-v8a", "armeabi-v7a", "x86_64", "x86"]
        
        for abi in abis:
            print(f"Building for Android {abi}...")
            
            abi_build_dir = self.build_dir / f"android-{abi}"
            abi_build_dir.mkdir(parents=True, exist_ok=True)
            
            cmake_args = self.get_cmake_args_common() + [
                f"-DCMAKE_INSTALL_PREFIX={self.output_dir / f'android-{abi}'}",
                f"-DCMAKE_TOOLCHAIN_FILE={toolchain_file}",
                f"-DANDROID_ABI={abi}",
                "-DANDROID_PLATFORM=android-21",
                "-DANDROID_STL=c++_shared",
                "-DSPM_ENABLE_SHARED=ON",
                "-DSPM_ENABLE_TCMALLOC=OFF",  # TCMalloc not typically used on Android
            ]
            
            self.run_command(["cmake"] + cmake_args + [str(self.source_dir)], cwd=abi_build_dir)
            self.run_command(["make", "-j", str(os.cpu_count() or 4)], cwd=abi_build_dir)
            self.run_command(["make", "install"], cwd=abi_build_dir)

    def build_windows(self):
        """Build for Windows"""
        self.install_dependencies_windows()
        
        # Try to find Visual Studio
        vs_versions = ["2022", "2019", "2017"]
        generator = None
        
        for vs_ver in vs_versions:
            try:
                self.run_command(["cmake", "--help"])
                generator = f"Visual Studio 16 {vs_ver}" if vs_ver == "2019" else f"Visual Studio 17 {vs_ver}"
                break
            except:
                continue
        
        if not generator:
            generator = "Visual Studio 16 2019"  # Default fallback
        
        # Build for x64
        cmake_args = self.get_cmake_args_common() + [
            f"-DCMAKE_INSTALL_PREFIX={self.output_dir / 'windows-x64'}",
            "-G", generator,
            "-A", "x64",
            "-DSPM_ENABLE_SHARED=ON",
            "-DSPM_ENABLE_TCMALLOC=OFF",  # Disable TCMalloc on Windows by default
        ]
        
        # Configure
        self.run_command(["cmake"] + cmake_args + [str(self.source_dir)])
        
        # Build
        self.run_command([
            "cmake", "--build", ".", 
            "--config", "Release", 
            "--parallel", str(os.cpu_count() or 4)
        ])
        
        # Install
        self.run_command([
            "cmake", "--build", ".", 
            "--config", "Release", 
            "--target", "install"
        ])

    def build(self):
        """Build for the target platform"""
        if self.target_platform == "linux":
            self.build_linux()
        elif self.target_platform == "macos":
            self.build_macos()
        elif self.target_platform == "ios":
            self.build_ios()
        elif self.target_platform == "android":
            self.build_android()
        elif self.target_platform == "windows":
            self.build_windows()
        else:
            raise SentencePieceBuildError(f"Unsupported platform: {self.target_platform}")
        
        print(f"✅ SentencePiece build completed for {self.target_platform}")
        print(f"Output directory: {self.output_dir}")

def main():
    parser = argparse.ArgumentParser(description="Build SentencePiece from source")
    parser.add_argument("--source", required=True, help="SentencePiece source directory")
    parser.add_argument("--output", required=True, help="Output directory for built libraries")
    parser.add_argument("--platform", choices=["linux", "macos", "ios", "android", "windows"], 
                       help="Target platform (auto-detected if not specified)")
    
    args = parser.parse_args()
    
    try:
        builder = SentencePieceBuilder(args.source, args.output, args.platform)
        builder.build()
    except SentencePieceBuildError as e:
        print(f"❌ Build failed: {e}")
        sys.exit(1)
    except KeyboardInterrupt:
        print("❌ Build interrupted by user")
        sys.exit(1)

if __name__ == "__main__":
    main() 