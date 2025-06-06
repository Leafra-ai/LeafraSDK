/*
 * C++ Bridge for ExecutTorch ExecuTorchLog
 * Auto-generated - DO NOT EDIT MANUALLY
 * 
 * This file provides a C++ interface to the ExecutTorch ExecuTorchLog Objective-C API
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef void* ExecuTorchLog_Handle;

// Log levels
typedef enum {
    ET_LOG_LEVEL_DEBUG = 'D',
    ET_LOG_LEVEL_INFO = 'I',
    ET_LOG_LEVEL_ERROR = 'E',
    ET_LOG_LEVEL_FATAL = 'F',
    ET_LOG_LEVEL_UNKNOWN = '?'
} ExecutorchLogLevel_t;

// Log sink callback type
typedef void (*LogSink_callback_t)(ExecutorchLogLevel_t level, double timestamp, 
                                   const char* filename, size_t line, const char* message);

// Log management
ExecuTorchLog_Handle ExecuTorchLog_getSharedInstance();
void ExecuTorchLog_addSink(ExecuTorchLog_Handle log, LogSink_callback_t callback);
void ExecuTorchLog_removeSink(ExecuTorchLog_Handle log, LogSink_callback_t callback);

#ifdef __cplusplus
}
#endif
