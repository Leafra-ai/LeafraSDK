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