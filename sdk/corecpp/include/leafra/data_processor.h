#pragma once

#include "types.h"

namespace leafra {

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
    class Impl;
    unique_ptr<Impl> pImpl;
};

} // namespace leafra 