#include "leafra/leafra_core.h"
#include "leafra/data_processor.h"
#include "leafra/math_utils.h"
#include "leafra/platform_utils.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>

namespace leafra {

// Private implementation class (PIMPL pattern)
class LeafraCore::Impl {
public:
    Config config;
    bool initialized = false;
    callback_t event_callback;
    std::unique_ptr<DataProcessor> data_processor;
    std::unique_ptr<MathUtils> math_utils;
    mutable std::mutex mutex;
    
    void log(const std::string& message) {
        if (event_callback) {
            event_callback("[LeafraCore] " + message);
        }
        if (config.debug_mode) {
            std::cout << "[LeafraCore Debug] " << message << std::endl;
        }
    }
};

LeafraCore::LeafraCore() : pImpl(std::make_unique<Impl>()) {
    pImpl->log("LeafraCore created");
}

LeafraCore::~LeafraCore() {
    if (pImpl && pImpl->initialized) {
        shutdown();
    }
}

ResultCode LeafraCore::initialize(const Config& config) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    
    if (pImpl->initialized) {
        pImpl->log("Already initialized");
        return ResultCode::SUCCESS;
    }
    
    pImpl->config = config;
    pImpl->log("Initializing LeafraCore with config: " + config.name);
    
    try {
        // Initialize data processor
        pImpl->data_processor = std::make_unique<DataProcessor>();
        auto result = pImpl->data_processor->initialize();
        if (result != ResultCode::SUCCESS) {
            pImpl->log("Failed to initialize data processor");
            return result;
        }
        
        // Initialize math utils
        pImpl->math_utils = std::make_unique<MathUtils>();
        result = pImpl->math_utils->initialize();
        if (result != ResultCode::SUCCESS) {
            pImpl->log("Failed to initialize math utils");
            return result;
        }
        
        pImpl->initialized = true;
        pImpl->log("LeafraCore initialized successfully");
        
        // Trigger initialization event
        if (pImpl->event_callback) {
            pImpl->event_callback("SDK initialized successfully");
        }
        
        return ResultCode::SUCCESS;
        
    } catch (const std::exception& e) {
        pImpl->log("Exception during initialization: " + std::string(e.what()));
        return ResultCode::ERROR_INITIALIZATION_FAILED;
    }
}

ResultCode LeafraCore::shutdown() {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    
    if (!pImpl->initialized) {
        return ResultCode::SUCCESS;
    }
    
    pImpl->log("Shutting down LeafraCore");
    
    // Cleanup resources
    pImpl->data_processor.reset();
    pImpl->math_utils.reset();
    pImpl->event_callback = nullptr;
    
    pImpl->initialized = false;
    pImpl->log("LeafraCore shutdown complete");
    
    return ResultCode::SUCCESS;
}

bool LeafraCore::is_initialized() const {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    return pImpl->initialized;
}

const Config& LeafraCore::get_config() const {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    return pImpl->config;
}

ResultCode LeafraCore::process_data(const data_buffer_t& input, data_buffer_t& output) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    
    if (!pImpl->initialized) {
        return ResultCode::ERROR_INITIALIZATION_FAILED;
    }
    
    if (input.empty()) {
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    try {
        // Use data processor to handle the data
        auto result = pImpl->data_processor->process(input, output);
        
        if (result == ResultCode::SUCCESS && pImpl->event_callback) {
            pImpl->event_callback("Data processed successfully, output size: " + 
                                std::to_string(output.size()));
        }
        
        return result;
        
    } catch (const std::exception& e) {
        pImpl->log("Exception during data processing: " + std::string(e.what()));
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

void LeafraCore::set_event_callback(callback_t callback) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    pImpl->event_callback = callback;
    pImpl->log("Event callback set");
}

std::string LeafraCore::get_version() {
    return std::to_string(LEAFRA_VERSION_MAJOR) + "." + 
           std::to_string(LEAFRA_VERSION_MINOR) + "." + 
           std::to_string(LEAFRA_VERSION_PATCH);
}

std::string LeafraCore::get_platform() {
#ifdef __APPLE__
    #ifdef TARGET_OS_IPHONE
        return "iOS";
    #else
        return "macOS";
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

shared_ptr<LeafraCore> LeafraCore::create() {
    return std::make_shared<LeafraCore>();
}

} // namespace leafra 