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
};

} // namespace leafra 