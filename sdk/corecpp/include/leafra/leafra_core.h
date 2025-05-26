#pragma once

#include "types.h"
#include <memory>

namespace leafra {

/**
 * @brief Main SDK interface class
 * 
 * This class provides the primary interface for the Leafra SDK.
 * It manages initialization, configuration, and core functionality.
 */
class LEAFRA_API LeafraCore {
public:
    /**
     * @brief Constructor
     */
    LeafraCore();
    
    /**
     * @brief Destructor
     */
    ~LeafraCore();
    
    /**
     * @brief Initialize the SDK with configuration
     * @param config Configuration parameters
     * @return ResultCode indicating success or failure
     */
    ResultCode initialize(const Config& config);
    
    /**
     * @brief Shutdown the SDK and cleanup resources
     * @return ResultCode indicating success or failure
     */
    ResultCode shutdown();
    
    /**
     * @brief Check if SDK is initialized
     * @return true if initialized, false otherwise
     */
    bool is_initialized() const;
    
    /**
     * @brief Get current configuration
     * @return Current configuration
     */
    const Config& get_config() const;
    
    /**
     * @brief Process data using the SDK
     * @param input Input data buffer
     * @param output Output data buffer
     * @return ResultCode indicating success or failure
     */
    ResultCode process_data(const data_buffer_t& input, data_buffer_t& output);
    
    /**
     * @brief Process user files through the SDK
     * @param file_paths Vector of file paths to process
     * @return ResultCode indicating success or failure
     */
    ResultCode process_user_files(const std::vector<std::string>& file_paths);
    
    /**
     * @brief Set event callback
     * @param callback Function to be called on events
     */
    void set_event_callback(callback_t callback);
    
    /**
     * @brief Get SDK version information
     * @return Version string
     */
    static std::string get_version();
    
    /**
     * @brief Get platform information
     * @return Platform string
     */
    static std::string get_platform();
    
    /**
     * @brief Create SDK instance
     * @return Shared pointer to LeafraCore instance
     */
    static shared_ptr<LeafraCore> create();

private:
    class Impl;
    unique_ptr<Impl> pImpl;
    
    // Non-copyable
    LeafraCore(const LeafraCore&) = delete;
    LeafraCore& operator=(const LeafraCore&) = delete;
    
    // Movable
    LeafraCore(LeafraCore&&) = default;
    LeafraCore& operator=(LeafraCore&&) = default;
};

} // namespace leafra 