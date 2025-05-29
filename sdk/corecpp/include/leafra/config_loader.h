#pragma once

#include "types.h"
#include <string>

namespace leafra {

/**
 * @brief Configuration loader utility for the LeafraSDK
 * 
 * This utility provides functionality to load configuration from JSON files
 * and populate the Config struct with appropriate values for SDK initialization.
 */
class LEAFRA_API ConfigLoader {
public:
    ConfigLoader() = default;
    ~ConfigLoader() = default;
    
    /**
     * @brief Load configuration from a JSON file
     * @param config_file_path Path to the JSON configuration file
     * @param config Reference to Config struct to populate
     * @return ResultCode indicating success or failure
     */
    static ResultCode load_from_file(const std::string& config_file_path, Config& config);
    
    /**
     * @brief Load configuration from a JSON string
     * @param json_string JSON configuration as string
     * @param config Reference to Config struct to populate
     * @return ResultCode indicating success or failure
     */
    static ResultCode load_from_string(const std::string& json_string, Config& config);
    
    /**
     * @brief Get default configuration file path
     * @return Default path to configuration file
     */
    static std::string get_default_config_path();
    
    /**
     * @brief Check if configuration file exists
     * @param config_file_path Path to configuration file
     * @return True if file exists and is readable
     */
    static bool config_file_exists(const std::string& config_file_path);
    
    /**
     * @brief Save current configuration to JSON file
     * @param config_file_path Path where to save the configuration
     * @param config Config struct to save
     * @return ResultCode indicating success or failure
     */
    static ResultCode save_to_file(const std::string& config_file_path, const Config& config);
    
    /**
     * @brief Convert configuration to JSON string
     * @param config Config struct to convert
     * @return JSON string representation
     */
    static std::string to_json_string(const Config& config);
    
private:
    // Helper methods for parsing specific sections
    static void parse_sdk_section(const std::string& json, Config& config);
    static void parse_chunking_section(const std::string& json, Config& config);
    static void parse_logging_section(const std::string& json, Config& config);
    
    // Helper methods for converting enums
    static ChunkSizeUnit parse_chunk_size_unit(const std::string& unit_str);
    static TokenApproximationMethod parse_token_method(const std::string& method_str);
    static std::string chunk_size_unit_to_string(ChunkSizeUnit unit);
    static std::string token_method_to_string(TokenApproximationMethod method);
    
    // JSON parsing helpers (simple implementation without external dependencies)
    static std::string extract_string_value(const std::string& json, const std::string& key);
    static int extract_int_value(const std::string& json, const std::string& key);
    static double extract_double_value(const std::string& json, const std::string& key);
    static bool extract_bool_value(const std::string& json, const std::string& key);
    
    // Validation helpers
    static bool validate_config(const Config& config);
    static std::string read_file_contents(const std::string& file_path);
    static bool write_file_contents(const std::string& file_path, const std::string& contents);
};

/**
 * @brief Convenience function to initialize SDK with configuration file
 * @param sdk Pointer to LeafraCore instance
 * @param config_file_path Path to configuration file (optional, uses default if empty)
 * @return ResultCode indicating success or failure
 */
LEAFRA_API ResultCode initialize_sdk_with_config(LeafraCore* sdk, const std::string& config_file_path = "");

} // namespace leafra 