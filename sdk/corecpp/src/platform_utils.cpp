#include "leafra/platform_utils.h"
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <unistd.h>
#include <climits>
#include <cstdlib>
#else
#include <unistd.h>
#include <climits>
#include <cstdlib>
#endif

#include <sys/stat.h>
#include <fstream>

namespace leafra {

std::string PlatformUtils::get_platform_name() {
#ifdef __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE
        return "iOS";
    #elif TARGET_OS_MAC
        return "macOS";
    #else
        return "Apple";
    #endif
#elif defined(__ANDROID__)
    return "Android";
#elif defined(_WIN32)
    return "Windows";
#elif defined(__linux__)
    return "Linux";
#else
    return "Unknown";
#endif
}

std::string PlatformUtils::get_architecture() {
#if defined(__x86_64__) || defined(_M_X64)
    return "x86_64";
#elif defined(__i386) || defined(_M_IX86)
    return "x86";
#elif defined(__aarch64__) || defined(_M_ARM64)
    return "arm64";
#elif defined(__arm__) || defined(_M_ARM)
    return "arm";
#else
    return "unknown";
#endif
}

bool PlatformUtils::is_mobile_platform() {
#if defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE
        return true;
    #else
        return false;
    #endif
#elif defined(__ANDROID__)
    return true;
#else
    return false;
#endif
}

bool PlatformUtils::is_desktop_platform() {
    return !is_mobile_platform();
}

std::string PlatformUtils::get_platform_info() {
    return get_platform_name() + " (" + get_architecture() + ")";
}

// Path utilities implementation

char PlatformUtils::get_path_separator() {
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

std::string PlatformUtils::join_paths(const std::string& path1, const std::string& path2) {
    if (path1.empty()) return path2;
    if (path2.empty()) return path1;
    
    char sep = get_path_separator();
    
    // Check if path1 already ends with separator
    if (path1.back() == '/' || path1.back() == '\\') {
        return path1 + path2;
    } else {
        return path1 + sep + path2;
    }
}

bool PlatformUtils::file_exists(const std::string& file_path) {
    if (file_path.empty()) return false;
    
#ifdef _WIN32
    return _access(file_path.c_str(), 0) == 0;
#else
    return access(file_path.c_str(), F_OK) == 0;
#endif
}

std::string PlatformUtils::get_executable_directory() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    DWORD size = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (size == 0 || size == MAX_PATH) {
        return "";
    }
    
    std::string exe_path(buffer);
    size_t last_slash = exe_path.find_last_of("\\/");
    if (last_slash != std::string::npos) {
        return exe_path.substr(0, last_slash);
    }
    return "";
    
#elif defined(__APPLE__)
    char buffer[PATH_MAX];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0) {
        char resolved_path[PATH_MAX];
        if (realpath(buffer, resolved_path) != nullptr) {
            std::string exe_path(resolved_path);
            size_t last_slash = exe_path.find_last_of('/');
            if (last_slash != std::string::npos) {
                return exe_path.substr(0, last_slash);
            }
        }
    }
    return "";
    
#elif defined(__linux__)
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        std::string exe_path(buffer);
        size_t last_slash = exe_path.find_last_of('/');
        if (last_slash != std::string::npos) {
            return exe_path.substr(0, last_slash);
        }
    }
    return "";
    
#else
    return "";
#endif
}

std::string PlatformUtils::get_sdk_root_directory() {
    // Strategy 1: Check LEAFRA_SDK_ROOT environment variable
    const char* env_root = std::getenv("LEAFRA_SDK_ROOT");
    if (env_root && *env_root) {
        std::string root_path(env_root);
        if (file_exists(join_paths(root_path, "sdk"))) {
            return root_path;
        }
    }
    
    // Strategy 2: Look relative to executable directory
    std::string exe_dir = get_executable_directory();
    if (!exe_dir.empty()) {
        // Common patterns to check:
        std::vector<std::string> candidate_paths = {
            // If executable is in: sdk/corecpp/build/ -> SDK root is ../../../
            join_paths(join_paths(join_paths(exe_dir, ".."), ".."), ".."),
            // If executable is in: build/ -> SDK root is ../
            join_paths(exe_dir, ".."),
            // If executable is in: sdk/corecpp/sdkcmdapp/ -> SDK root is ../../../
            join_paths(join_paths(join_paths(exe_dir, ".."), ".."), ".."),
            // If executable is in install/bin/ -> SDK root might be ../
            join_paths(exe_dir, ".."),
            // Current directory
            exe_dir
        };
        
        for (const auto& candidate : candidate_paths) {
            // Check if this looks like SDK root by looking for characteristic files/dirs
            std::string sdk_dir = join_paths(candidate, "sdk");
            std::string thirdparty_dir = join_paths(candidate, "thirdparty");
            
            if (file_exists(sdk_dir) || file_exists(thirdparty_dir)) {
                return candidate;
            }
        }
    }
    
    // Strategy 3: Check some common installation paths
    std::vector<std::string> common_paths = {
        "/usr/local/share/leafra",
        "/opt/leafra",
        "C:\\Program Files\\LeafraSDK",
        "C:\\Program Files (x86)\\LeafraSDK"
    };
    
    for (const auto& path : common_paths) {
        if (file_exists(path)) {
            return path;
        }
    }
    
    return "";
}

std::string PlatformUtils::resolve_sdk_resource_path(const std::string& relative_path) {
    if (relative_path.empty()) return "";
    
    std::string sdk_root = get_sdk_root_directory();
    if (sdk_root.empty()) {
        return "";
    }
    
    std::string full_path = join_paths(sdk_root, relative_path);
    
    // Verify the file exists
    if (file_exists(full_path)) {
        return full_path;
    }
    
    return "";
}

// TokenizerConfig method implementation
bool TokenizerConfig::resolve_model_path() {
    // If already resolved and exists, keep it
    if (!sentencepiece_model_path.empty() && PlatformUtils::file_exists(sentencepiece_model_path)) {
        return true;
    }
    
    // Look for sentencepiece.bpe.model in sdk/corecpp/third_party/models/embedding/{model_name}/ directory
    std::string model_path = "sdk/corecpp/third_party/models/embedding/" + model_name + "/sentencepiece.bpe.model";
    std::string resolved_path = PlatformUtils::resolve_sdk_resource_path(model_path);
    
    if (!resolved_path.empty()) {
        sentencepiece_model_path = resolved_path;
        
        // Also resolve the tokenizer_config.json file in the same directory
        std::string json_path = "sdk/corecpp/third_party/models/embedding/" + model_name + "/tokenizer_config.json";
        std::string resolved_json_path = PlatformUtils::resolve_sdk_resource_path(json_path);
        
        if (!resolved_json_path.empty()) {
            sentencepiece_json_path = resolved_json_path;
        } else {
            // Set empty if JSON file not found (it might be optional)
            sentencepiece_json_path = "";
        }
        
        return true;
    }
    
    // Fallback: set default paths even if they don't exist
    sentencepiece_model_path = "";
    sentencepiece_json_path = "";
    return false;
}

} // namespace leafra 