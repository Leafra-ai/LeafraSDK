/*
 * C++ Bridge Implementation for ExecutTorch ExecuTorchLog
 * Auto-generated - DO NOT EDIT MANUALLY
 */

#import <Foundation/Foundation.h>
#import <executorch/ExecuTorchLog.h>
#include "ExecuTorchLog_Bridge.h"

#ifdef __cplusplus
extern "C" {
#endif

// Bridge implementation for ExecuTorchLog

@interface LogSinkBridge : NSObject<ExecuTorchLogSink>
@property (nonatomic, assign) LogSink_callback_t callback;
@end

@implementation LogSinkBridge
- (void)logWithLevel:(ExecuTorchLogLevel)level
           timestamp:(NSTimeInterval)timestamp
            filename:(NSString *)filename
                line:(NSUInteger)line
             message:(NSString *)message {
    if (self.callback) {
        ExecutorchLogLevel_t cLevel = (ExecutorchLogLevel_t)level;
        self.callback(cLevel, timestamp, [filename UTF8String], line, [message UTF8String]);
    }
}
@end

// Global storage for sink bridges
static NSMutableDictionary<NSValue*, LogSinkBridge*>* sinkBridges = nil;

ExecuTorchLog_Handle ExecuTorchLog_getSharedInstance() {
    return (__bridge void*)[ExecuTorchLog sharedLog];
}

void ExecuTorchLog_addSink(ExecuTorchLog_Handle log, LogSink_callback_t callback) {
    if (!sinkBridges) {
        sinkBridges = [[NSMutableDictionary alloc] init];
    }
    
    LogSinkBridge* bridge = [[LogSinkBridge alloc] init];
    bridge.callback = callback;
    
    NSValue* key = [NSValue valueWithPointer:(const void*)callback];
    sinkBridges[key] = bridge;
    
    ExecuTorchLog* objcLog = (__bridge ExecuTorchLog*)log;
    [objcLog addSink:bridge];
}

void ExecuTorchLog_removeSink(ExecuTorchLog_Handle log, LogSink_callback_t callback) {
    if (!sinkBridges) return;
    
    NSValue* key = [NSValue valueWithPointer:(const void*)callback];
    LogSinkBridge* bridge = sinkBridges[key];
    if (bridge) {
        ExecuTorchLog* objcLog = (__bridge ExecuTorchLog*)log;
        [objcLog removeSink:bridge];
        [sinkBridges removeObjectForKey:key];
    }
}

#ifdef __cplusplus
}
#endif
