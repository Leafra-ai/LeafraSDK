#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>

// Platform-specific macros
#ifdef _WIN32
    #ifdef LEAFRA_EXPORTS
        #define LEAFRA_API __declspec(dllexport)
    #elif defined(LEAFRA_SHARED)
        #define LEAFRA_API __declspec(dllimport)
    #else
        #define LEAFRA_API
    #endif
#else
    #ifdef LEAFRA_EXPORTS
        #define LEAFRA_API __attribute__((visibility("default")))
    #else
        #define LEAFRA_API
    #endif
#endif

// Version macros
#define LEAFRA_VERSION_MAJOR 1
#define LEAFRA_VERSION_MINOR 0
#define LEAFRA_VERSION_PATCH 0

namespace leafra {

// Forward declarations
class DataProcessor;
class MathUtils;

// Basic types
using byte_t = uint8_t;
using data_buffer_t = std::vector<byte_t>;
using callback_t = std::function<void(const std::string&)>;

// Result types
enum class ResultCode : int32_t {
    SUCCESS = 0,
    ERROR_INVALID_PARAMETER = -1,
    ERROR_INITIALIZATION_FAILED = -2,
    ERROR_PROCESSING_FAILED = -3,
    ERROR_NOT_IMPLEMENTED = -4,
    ERROR_OUT_OF_MEMORY = -5
};

// Configuration structure
struct Config {
    std::string name;
    std::string version;
    bool debug_mode = false;
    int32_t max_threads = 4;
    size_t buffer_size = 1024;
};

// Data structures
struct Point2D {
    double x = 0.0;
    double y = 0.0;
    
    Point2D() = default;
    Point2D(double x_val, double y_val) : x(x_val), y(y_val) {}
};

struct Point3D {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    
    Point3D() = default;
    Point3D(double x_val, double y_val, double z_val) : x(x_val), y(y_val), z(z_val) {}
};

struct Matrix3x3 {
    double data[9] = {0.0};
    
    Matrix3x3() {
        for (int i = 0; i < 9; ++i) {
            data[i] = 0.0;
        }
    }
    
    explicit Matrix3x3(const double* values) {
        for (int i = 0; i < 9; ++i) {
            data[i] = values[i];
        }
    }
    
    double& operator()(int row, int col) {
        return data[row * 3 + col];
    }
    
    const double& operator()(int row, int col) const {
        return data[row * 3 + col];
    }
};

// Event types
enum class EventType : int32_t {
    INITIALIZATION_COMPLETE = 0,
    DATA_PROCESSED = 1,
    ERROR_OCCURRED = 2,
    CUSTOM_EVENT = 100
};

struct Event {
    EventType type;
    std::string message;
    int64_t timestamp;
    data_buffer_t data;
    
    Event(EventType t, const std::string& msg) 
        : type(t), message(msg), timestamp(0) {}
};

// Smart pointer aliases
template<typename T>
using unique_ptr = std::unique_ptr<T>;

template<typename T>
using shared_ptr = std::shared_ptr<T>;

// Utility functions
LEAFRA_API const char* result_code_to_string(ResultCode code);
LEAFRA_API std::string get_version_string();
LEAFRA_API int64_t get_current_timestamp();

} // namespace leafra 