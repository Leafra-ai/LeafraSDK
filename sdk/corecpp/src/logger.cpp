#include "leafra/logger.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cstring>

// Platform-specific includes
#ifdef __APPLE__
    #include <os/log.h>
    #include <TargetConditionals.h>
#elif defined(__ANDROID__)
    #include <android/log.h>
#elif defined(_WIN32)
    #include <windows.h>
    #include <debugapi.h>
#endif

namespace leafra {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logLevel = level;
}

LogLevel Logger::getLogLevel() const {
    return m_logLevel;
}

void Logger::setShowFileInfo(bool show) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_showFileInfo = show;
}

void Logger::log(LogLevel level, const std::string& message, const char* file, int line) {
    if (level < m_logLevel) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string formattedMessage = formatMessage(level, message, file, line);
    platformLog(level, formattedMessage);
}

void Logger::debug(const std::string& message, const char* file, int line) {
    log(LogLevel::LEAFRA_DEBUG, message, file, line);
}

void Logger::info(const std::string& message, const char* file, int line) {
    log(LogLevel::LEAFRA_INFO, message, file, line);
}

void Logger::warning(const std::string& message, const char* file, int line) {
    log(LogLevel::LEAFRA_WARNING, message, file, line);
}

void Logger::error(const std::string& message, const char* file, int line) {
    log(LogLevel::LEAFRA_ERROR, message, file, line);
}

void Logger::platformLog(LogLevel level, const std::string& message) {
#ifdef __APPLE__
    // iOS/macOS - Use os_log
    os_log_type_t osLogType;
    switch (level) {
        case LogLevel::LEAFRA_DEBUG:   osLogType = OS_LOG_TYPE_DEBUG; break;
        case LogLevel::LEAFRA_INFO:    osLogType = OS_LOG_TYPE_INFO; break;
        case LogLevel::LEAFRA_WARNING: osLogType = OS_LOG_TYPE_DEFAULT; break;
        case LogLevel::LEAFRA_ERROR:   osLogType = OS_LOG_TYPE_ERROR; break;
        default:                       osLogType = OS_LOG_TYPE_DEFAULT; break;
    }
    
    os_log_t log = os_log_create("com.leafra.sdk", "LeafraSDK");
    os_log_with_type(log, osLogType, "%{public}s", message.c_str());
    
#elif defined(__ANDROID__)
    // Android - Use __android_log_print
    android_LogPriority priority;
    switch (level) {
        case LogLevel::LEAFRA_DEBUG:   priority = ANDROID_LOG_DEBUG; break;
        case LogLevel::LEAFRA_INFO:    priority = ANDROID_LOG_INFO; break;
        case LogLevel::LEAFRA_WARNING: priority = ANDROID_LOG_WARN; break;
        case LogLevel::LEAFRA_ERROR:   priority = ANDROID_LOG_ERROR; break;
        default:                       priority = ANDROID_LOG_INFO; break;
    }
    
    __android_log_print(priority, "LeafraSDK", "%s", message.c_str());
    
#elif defined(_WIN32)
    // Windows - Use OutputDebugStringA
    std::string windowsMessage = message + "\n";
    OutputDebugStringA(windowsMessage.c_str());
    
    // Also output to console for console apps
    std::cout << message << std::endl;
    
#else
    // Linux/Other - Use standard output
    std::cout << message << std::endl;
#endif
}

std::string Logger::formatMessage(LogLevel level, const std::string& message, 
                                 const char* file, int line) {
    std::ostringstream oss;
    
    // Timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    oss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    // Log level
    oss << " [" << levelToString(level) << "]";
    
    // File and line (if enabled and available)
    if (m_showFileInfo && file && line > 0) {
        const char* filename = std::strrchr(file, '/');
        if (!filename) filename = std::strrchr(file, '\\');
        if (filename) filename++; else filename = file;
        
        oss << " [" << filename << ":" << line << "]";
    }
    
    // Message
    oss << " " << message;
    
    return oss.str();
}

const char* Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::LEAFRA_DEBUG:   return "DEBUG";
        case LogLevel::LEAFRA_INFO:    return "INFO ";
        case LogLevel::LEAFRA_WARNING: return "WARN ";
        case LogLevel::LEAFRA_ERROR:   return "ERROR";
        default:                       return "UNKN ";
    }
}

} // namespace leafra 