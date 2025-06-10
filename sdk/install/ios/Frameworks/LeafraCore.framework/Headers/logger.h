#pragma once

#include "types.h"
#include <string>
#include <sstream>
#include <mutex>

namespace leafra {

// Log levels
enum class LogLevel : int32_t {
    LEAFRA_DEBUG = 0,
    LEAFRA_INFO = 1,
    LEAFRA_WARNING = 2,
    LEAFRA_ERROR = 3,
    LEAFRA_NONE = 4  // Disable all logging
};

// Logger class
class LEAFRA_API Logger {
public:
    static Logger& getInstance();
    
    // Set minimum log level
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const;
    
    // Enable/disable file and line info
    void setShowFileInfo(bool show);
    
    // Main logging function
    void log(LogLevel level, const std::string& message, 
             const char* file = nullptr, int line = 0);
    
    // Convenience methods
    void debug(const std::string& message, const char* file = nullptr, int line = 0);
    void info(const std::string& message, const char* file = nullptr, int line = 0);
    void warning(const std::string& message, const char* file = nullptr, int line = 0);
    void error(const std::string& message, const char* file = nullptr, int line = 0);
    
private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    void platformLog(LogLevel level, const std::string& message);
    std::string formatMessage(LogLevel level, const std::string& message, 
                             const char* file, int line);
    const char* levelToString(LogLevel level);
    
    LogLevel m_logLevel = LogLevel::LEAFRA_INFO;
    bool m_showFileInfo = true;
    std::mutex m_mutex;
};

// Convenience macros
#define LEAFRA_LOG_DEBUG(msg) \
    leafra::Logger::getInstance().debug(msg, __FILE__, __LINE__)

#define LEAFRA_LOG_INFO(msg) \
    leafra::Logger::getInstance().info(msg, __FILE__, __LINE__)

#define LEAFRA_LOG_WARNING(msg) \
    leafra::Logger::getInstance().warning(msg, __FILE__, __LINE__)

#define LEAFRA_LOG_ERROR(msg) \
    leafra::Logger::getInstance().error(msg, __FILE__, __LINE__)

// Stream-style logging macros
#define LEAFRA_LOG_STREAM(level) \
    leafra::LogStream(leafra::Logger::getInstance(), level, __FILE__, __LINE__)

#define LEAFRA_DEBUG() LEAFRA_LOG_STREAM(leafra::LogLevel::LEAFRA_DEBUG)
#define LEAFRA_INFO() LEAFRA_LOG_STREAM(leafra::LogLevel::LEAFRA_INFO)
#define LEAFRA_WARNING() LEAFRA_LOG_STREAM(leafra::LogLevel::LEAFRA_WARNING)
#define LEAFRA_ERROR() LEAFRA_LOG_STREAM(leafra::LogLevel::LEAFRA_ERROR)

// Stream helper class
class LEAFRA_API LogStream {
public:
    LogStream(Logger& logger, LogLevel level, const char* file, int line)
        : m_logger(logger), m_level(level), m_file(file), m_line(line) {}
    
    ~LogStream() {
        m_logger.log(m_level, m_stream.str(), m_file, m_line);
    }
    
    template<typename T>
    LogStream& operator<<(const T& value) {
        m_stream << value;
        return *this;
    }
    
private:
    Logger& m_logger;
    LogLevel m_level;
    const char* m_file;
    int m_line;
    std::ostringstream m_stream;
};

} // namespace leafra 