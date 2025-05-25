#include "leafra/data_processor.h"
#include <algorithm>
#include <numeric>

namespace leafra {

DataProcessor::DataProcessor() = default;

DataProcessor::~DataProcessor() = default;

ResultCode DataProcessor::initialize() {
    // Initialize any resources needed for data processing
    return ResultCode::SUCCESS;
}

ResultCode DataProcessor::process(const data_buffer_t& input, data_buffer_t& output) {
    if (input.empty()) {
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    try {
        // Simple data processing example: apply a transformation to each byte
        output.clear();
        output.reserve(input.size());
        
        // Example transformation: apply a simple filter/transformation
        for (size_t i = 0; i < input.size(); ++i) {
            byte_t processed_byte = static_cast<byte_t>((input[i] + 1) % 256);
            output.push_back(processed_byte);
        }
        
        return ResultCode::SUCCESS;
        
    } catch (const std::exception&) {
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

ResultCode DataProcessor::process_advanced(const data_buffer_t& input, 
                                         data_buffer_t& output,
                                         const ProcessingOptions& options) {
    if (input.empty()) {
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    try {
        output.clear();
        
        switch (options.algorithm) {
            case ProcessingAlgorithm::SIMPLE_TRANSFORM:
                return process(input, output);
                
            case ProcessingAlgorithm::REVERSE:
                output = input;
                std::reverse(output.begin(), output.end());
                break;
                
            case ProcessingAlgorithm::ACCUMULATE:
                {
                    byte_t accumulator = 0;
                    for (byte_t byte : input) {
                        accumulator = static_cast<byte_t>((accumulator + byte) % 256);
                        output.push_back(accumulator);
                    }
                }
                break;
                
            case ProcessingAlgorithm::FILTER:
                {
                    // Filter out bytes below threshold
                    byte_t threshold = options.threshold;
                    for (byte_t byte : input) {
                        if (byte >= threshold) {
                            output.push_back(byte);
                        }
                    }
                }
                break;
                
            default:
                return ResultCode::ERROR_NOT_IMPLEMENTED;
        }
        
        return ResultCode::SUCCESS;
        
    } catch (const std::exception&) {
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

size_t DataProcessor::get_processed_count() const {
    return processed_count_;
}

void DataProcessor::reset_statistics() {
    processed_count_ = 0;
}

} // namespace leafra 