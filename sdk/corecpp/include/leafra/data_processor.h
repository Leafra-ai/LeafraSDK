#pragma once

#include "types.h"

namespace leafra {

/**
 * @brief Processing algorithm types
 */
enum class ProcessingAlgorithm {
    SIMPLE_TRANSFORM,
    REVERSE,
    ACCUMULATE,
    FILTER
};

/**
 * @brief Processing options structure
 */
struct ProcessingOptions {
    ProcessingAlgorithm algorithm = ProcessingAlgorithm::SIMPLE_TRANSFORM;
    byte_t threshold = 128;
    size_t buffer_size = 1024;
};

/**
 * @brief Data processing utility class
 */
class LEAFRA_API DataProcessor {
public:
    DataProcessor();
    ~DataProcessor();
    
    /**
     * @brief Initialize the data processor
     * @return ResultCode indicating success or failure
     */
    ResultCode initialize();
    
    /**
     * @brief Process input data and generate output
     * @param input Input data buffer
     * @param output Output data buffer
     * @return ResultCode indicating success or failure
     */
    ResultCode process(const data_buffer_t& input, data_buffer_t& output);
    
    /**
     * @brief Advanced processing with options
     * @param input Input data buffer
     * @param output Output data buffer
     * @param options Processing options
     * @return ResultCode indicating success or failure
     */
    ResultCode process_advanced(const data_buffer_t& input, 
                               data_buffer_t& output,
                               const ProcessingOptions& options);
    
    /**
     * @brief Get number of processed items
     * @return Number of processed items
     */
    size_t get_processed_count() const;
    
    /**
     * @brief Reset processing statistics
     */
    void reset_statistics();
    
    /**
     * @brief Set processing parameters
     * @param buffer_size Size of internal buffer
     * @return ResultCode indicating success or failure
     */
    ResultCode set_buffer_size(size_t buffer_size);
    
    /**
     * @brief Get current buffer size
     * @return Current buffer size
     */
    size_t get_buffer_size() const;
    
private:
    size_t processed_count_ = 0;
    size_t buffer_size_ = 1024;
};

} // namespace leafra 