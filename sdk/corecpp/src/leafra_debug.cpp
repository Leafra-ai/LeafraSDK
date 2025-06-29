#include "leafra/leafra_debug.h"
#include "leafra/logger.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdio>

// Platform-specific includes for high-resolution timing
#ifdef _WIN32
    #include <windows.h>
#elif defined(__APPLE__)
    #include <mach/mach_time.h>
    #include <mach/mach.h>
    #include <TargetConditionals.h>
#elif defined(__ANDROID__)
    #include <time.h>
    #include <android/log.h>
#else
    #include <time.h>
    #include <sys/time.h>
#endif

namespace leafra {
namespace debug {

// Global debug state
static bool g_debug_enabled = false;

void set_debug_enabled(bool enabled) {
    g_debug_enabled = enabled;
}

bool is_debug_enabled() {
#ifdef NDEBUG
    // In release builds, only enabled if explicitly set
    return g_debug_enabled;
#else
    // In debug builds, always enabled or explicitly controlled
    return true || g_debug_enabled;
#endif
}

// Cross-platform high-resolution timer implementation
namespace timer {

#ifdef _WIN32
    // Windows implementation using QueryPerformanceCounter
    static double get_frequency() {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        return static_cast<double>(freq.QuadPart);
    }
    
    static double frequency = get_frequency();
    
    TimePoint now() {
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        return TimePoint{static_cast<double>(counter.QuadPart) / frequency};
    }

#elif defined(__APPLE__)
    // macOS/iOS implementation using mach_absolute_time
    static double get_timebase() {
        mach_timebase_info_data_t info;
        mach_timebase_info(&info);
        return static_cast<double>(info.numer) / static_cast<double>(info.denom) / 1e9;
    }
    
    static double timebase = get_timebase();
    
    TimePoint now() {
        uint64_t time = mach_absolute_time();
        return TimePoint{static_cast<double>(time) * timebase};
    }

#else
    // Linux/Android implementation using clock_gettime
    TimePoint now() {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return TimePoint{static_cast<double>(ts.tv_sec) + static_cast<double>(ts.tv_nsec) / 1e9};
    }
#endif

double elapsed_seconds(const TimePoint& start, const TimePoint& end) {
    return end.timestamp - start.timestamp;
}

double elapsed_milliseconds(const TimePoint& start, const TimePoint& end) {
    return (end.timestamp - start.timestamp) * 1000.0;
}

double elapsed_microseconds(const TimePoint& start, const TimePoint& end) {
    return (end.timestamp - start.timestamp) * 1000000.0;
}

} // namespace timer

// Scoped timer implementation
ScopedTimer::ScopedTimer(const std::string& name) 
    : name_(name), start_time_(timer::now()) {
    if (is_debug_enabled()) {
        debug_log("TIMER_START", name_);
    }
}

ScopedTimer::~ScopedTimer() {
    if (is_debug_enabled()) {
        auto end_time = timer::now();
        double elapsed_ms = timer::elapsed_milliseconds(start_time_, end_time);
        std::ostringstream oss;
        oss << name_ << " completed in " << std::fixed << std::setprecision(3) << elapsed_ms << "ms";
        debug_log("TIMER_END", oss.str());
    }
}

double ScopedTimer::elapsed_milliseconds() const {
    auto current_time = timer::now();
    return timer::elapsed_milliseconds(start_time_, current_time);
}

// Debug logging implementation
void debug_log(const std::string& category, const std::string& message) {
    if (!is_debug_enabled()) {
        return;
    }
    
    std::ostringstream oss;
    oss << "[DEBUG:" << category << "] " << message;
    std::string log_message = oss.str();
    
    // Use centralized logger system instead of platform-specific implementations
    Logger::getInstance().debug(log_message);
}

void debug_log_performance(const std::string& operation, 
                          size_t input_size, 
                          size_t output_count, 
                          double duration_ms) {
    if (!is_debug_enabled()) {
        return;
    }
    
    std::ostringstream oss;
    oss << operation << " - Input: " << input_size << " chars, Output: " << output_count 
        << " chunks, Duration: " << std::fixed << std::setprecision(3) << duration_ms << "ms";
    
    if (input_size > 0) {
        double chars_per_second = (static_cast<double>(input_size) / duration_ms) * 1000.0;
        oss << ", Speed: " << std::fixed << std::setprecision(0) << chars_per_second << " chars/sec";
    }
    
    debug_log("PERFORMANCE", oss.str());
}

void debug_log_chunking_details(const std::string& phase,
                               size_t chunk_index,
                               size_t start_pos,
                               size_t end_pos,
                               size_t estimated_tokens,
                               size_t target_tokens) {
    if (!is_debug_enabled()) {
        return;
    }
    
    std::ostringstream oss;
    oss << phase << " - Chunk #" << (chunk_index + 1) 
        << " [" << start_pos << "-" << end_pos << "] "
        << "(" << (end_pos - start_pos) << " chars, "
        << estimated_tokens << "/" << target_tokens << " tokens)";
    
    debug_log("CHUNKING", oss.str());
}

} // namespace debug
} // namespace leafra 