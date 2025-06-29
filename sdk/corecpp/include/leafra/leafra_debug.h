#pragma once

#include <string>

namespace leafra {
namespace debug {

// Enable/disable debug functionality
// NOTE: This separate debug mechanism exists for performance reasons.
// Debug logging involves expensive operations (string formatting, calculations, etc.)
// that we want to avoid entirely when debug is disabled, rather than just filtering
// at the Logger level. This provides a fast early-exit before any processing occurs.
void set_debug_enabled(bool enabled);
bool is_debug_enabled();

// High-resolution timing utilities
namespace timer {
    
    struct TimePoint {
        double timestamp; // Seconds since some reference point
    };
    
    // Get current high-resolution timestamp
    TimePoint now();
    
    // Calculate elapsed time between two points
    double elapsed_seconds(const TimePoint& start, const TimePoint& end);
    double elapsed_milliseconds(const TimePoint& start, const TimePoint& end);
    double elapsed_microseconds(const TimePoint& start, const TimePoint& end);
    
} // namespace timer

// RAII timer for automatic measurement and logging
class ScopedTimer {
public:
    explicit ScopedTimer(const std::string& name);
    ~ScopedTimer();
    
    // Get current elapsed time without stopping the timer
    double elapsed_milliseconds() const;
    
private:
    std::string name_;
    timer::TimePoint start_time_;
};

// Debug logging functions
void debug_log(const std::string& category, const std::string& message);

void debug_log_performance(const std::string& operation, 
                          size_t input_size, 
                          size_t output_count, 
                          double duration_ms);

void debug_log_chunking_details(const std::string& phase,
                               size_t chunk_index,
                               size_t start_pos,
                               size_t end_pos,
                               size_t estimated_tokens,
                               size_t target_tokens);

// Convenience macros for debug builds
#ifndef NDEBUG
    #define LEAFRA_DEBUG_LOG(category, message) \
        leafra::debug::debug_log(category, message)
    
    #define LEAFRA_DEBUG_TIMER(name) \
        leafra::debug::ScopedTimer _timer(name)
    
    #define LEAFRA_DEBUG_PERFORMANCE(op, input, output, duration) \
        leafra::debug::debug_log_performance(op, input, output, duration)
    
    #define LEAFRA_DEBUG_CHUNKING(phase, idx, start, end, tokens, target) \
        leafra::debug::debug_log_chunking_details(phase, idx, start, end, tokens, target)
#else
    #define LEAFRA_DEBUG_LOG(category, message) do {} while(0)
    #define LEAFRA_DEBUG_TIMER(name) do {} while(0)
    #define LEAFRA_DEBUG_PERFORMANCE(op, input, output, duration) do {} while(0)
    #define LEAFRA_DEBUG_CHUNKING(phase, idx, start, end, tokens, target) do {} while(0)
#endif

} // namespace debug
} // namespace leafra 