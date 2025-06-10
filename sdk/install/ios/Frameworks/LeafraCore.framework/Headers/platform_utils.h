#pragma once

#include "types.h"

namespace leafra {

/**
 * @brief Platform-specific utility functions
 */
class LEAFRA_API PlatformUtils {
public:
    /**
     * @brief Get current platform name
     * @return Platform name string
     */
    static std::string get_platform_name();
    
    /**
     * @brief Get platform architecture
     * @return Architecture string (x86_64, arm64, etc.)
     */
    static std::string get_architecture();
    
    /**
     * @brief Check if running on mobile platform
     * @return true if mobile (iOS, Android), false otherwise
     */
    static bool is_mobile_platform();
    
    /**
     * @brief Check if running on desktop platform
     * @return true if desktop (macOS, Windows, Linux), false otherwise
     */
    static bool is_desktop_platform();
    
    /**
     * @brief Get comprehensive platform information
     * @return Platform info string combining name and architecture
     */
    static std::string get_platform_info();
    
    /**
     * @brief Get platform version
     * @return Platform version string
     */
    static std::string get_platform_version();
    
    /**
     * @brief Get current timestamp in milliseconds
     * @return Timestamp
     */
    static int64_t get_timestamp_ms();
    
    /**
     * @brief Sleep for specified milliseconds
     * @param ms Milliseconds to sleep
     */
    static void sleep_ms(int32_t ms);
    
    /**
     * @brief Get number of CPU cores
     * @return Number of cores
     */
    static int32_t get_cpu_cores();
    
    // Path utilities
    
    /**
     * @brief Get the directory where the current executable is located
     * @return Executable directory path, or empty string if unable to determine
     */
    static std::string get_executable_directory();
    
    /**
     * @brief Get the SDK root directory
     * 
     * This function attempts to locate the SDK root directory using several strategies:
     * 1. Check LEAFRA_SDK_ROOT environment variable
     * 2. Look for SDK structure relative to executable directory
     * 3. Check common installation paths
     * 
     * @return SDK root path, or empty string if not found
     */
    static std::string get_sdk_root_directory();
    
    /**
     * @brief Resolve a path relative to the SDK root
     * @param relative_path Path relative to SDK root (e.g., "thirdparty/models/file.model")
     * @return Absolute path to the resource, or empty string if SDK root not found
     */
    static std::string resolve_sdk_resource_path(const std::string& relative_path);
    
    /**
     * @brief Check if a file exists at the given path
     * @param file_path Path to check
     * @return true if file exists and is accessible, false otherwise
     */
    static bool file_exists(const std::string& file_path);
    
    /**
     * @brief Join two path components with appropriate path separator
     * @param path1 First path component
     * @param path2 Second path component  
     * @return Joined path
     */
    static std::string join_paths(const std::string& path1, const std::string& path2);
    
    /**
     * @brief Get the appropriate path separator for the current platform
     * @return Path separator character ('/' on Unix-like, '\\' on Windows)
     */
    static char get_path_separator();
};

} // namespace leafra 