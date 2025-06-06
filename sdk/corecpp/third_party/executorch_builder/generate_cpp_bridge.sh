#!/bin/bash

# Script to generate C++ bridge files for ExecutTorch XCFrameworks
# This script creates C++ headers and Objective-C++ implementation files
# to allow calling ExecutTorch functions from C++

set -e

BRIDGE_DIR="../executorch_c++_bridge"
XCFRAMEWORK_HEADERS="../prebuilt/executorch/apple/executorch.xcframework/ios-arm64/Headers/executorch"

echo "Generating C++ bridge files in $BRIDGE_DIR..."

# Create bridge directory if it doesn't exist
mkdir -p "$BRIDGE_DIR"

# List of header files to bridge
HEADERS=("ExecuTorch" "ExecuTorchError" "ExecuTorchLog" "ExecuTorchModule" "ExecuTorchTensor" "ExecuTorchValue")

# Generate bridge files for each header
for header in "${HEADERS[@]}"; do
    echo "Generating bridge for $header..."
    
    # Generate C++ header file
    cat > "$BRIDGE_DIR/${header}_Bridge.h" << EOF
/*
 * C++ Bridge for ExecutTorch $header
 * Auto-generated - DO NOT EDIT MANUALLY
 * 
 * This file provides a C++ interface to the ExecutTorch $header Objective-C API
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
typedef void* ${header}_Handle;

EOF

    # Generate Objective-C++ implementation file
    cat > "$BRIDGE_DIR/${header}_Bridge.mm" << EOF
/*
 * C++ Bridge Implementation for ExecutTorch $header
 * Auto-generated - DO NOT EDIT MANUALLY
 */

#import <Foundation/Foundation.h>
#import <executorch/$header.h>
#include "${header}_Bridge.h"

#ifdef __cplusplus
extern "C" {
#endif

EOF

    case $header in
        "ExecuTorch")
            # Main header just includes others, minimal bridge needed
            cat >> "$BRIDGE_DIR/${header}_Bridge.h" << EOF
// Main ExecutTorch header - includes all other headers
// Use specific component bridges for actual functionality

#ifdef __cplusplus
}
#endif
EOF
            cat >> "$BRIDGE_DIR/${header}_Bridge.mm" << EOF
// Main ExecutTorch bridge - minimal implementation
// Actual functionality is in component-specific bridges

#ifdef __cplusplus
}
#endif
EOF
            ;;
        
        "ExecuTorchError")
            cat >> "$BRIDGE_DIR/${header}_Bridge.h" << EOF
// ExecutTorch Error Domain
const char* ExecuTorchError_getDomain();

#ifdef __cplusplus
}
#endif
EOF
            cat >> "$BRIDGE_DIR/${header}_Bridge.mm" << EOF
const char* ExecuTorchError_getDomain() {
    return [ExecuTorchErrorDomain UTF8String];
}

#ifdef __cplusplus
}
#endif
EOF
            ;;
        
        "ExecuTorchLog")
            cat >> "$BRIDGE_DIR/${header}_Bridge.h" << EOF
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
EOF
            cat >> "$BRIDGE_DIR/${header}_Bridge.mm" << EOF
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
EOF
            ;;
        
        "ExecuTorchModule")
            cat >> "$BRIDGE_DIR/${header}_Bridge.h" << EOF
// Module load modes
typedef enum {
    ET_MODULE_LOAD_MODE_FILE = 0,
    ET_MODULE_LOAD_MODE_MMAP,
    ET_MODULE_LOAD_MODE_MMAP_USE_MLOCK,
    ET_MODULE_LOAD_MODE_MMAP_USE_MLOCK_IGNORE_ERRORS
} ExecutorchModuleLoadMode_t;

// Module verification levels
typedef enum {
    ET_VERIFICATION_MINIMAL = 0,
    ET_VERIFICATION_INTERNAL_CONSISTENCY
} ExecutorchVerification_t;

// Module management
ExecuTorchModule_Handle ExecuTorchModule_create(const char* filePath, ExecutorchModuleLoadMode_t loadMode);
ExecuTorchModule_Handle ExecuTorchModule_createWithDefaultMode(const char* filePath);
void ExecuTorchModule_destroy(ExecuTorchModule_Handle module);

// Module loading
bool ExecuTorchModule_loadWithVerification(ExecuTorchModule_Handle module, ExecutorchVerification_t verification, char** error);
bool ExecuTorchModule_load(ExecuTorchModule_Handle module, char** error);
bool ExecuTorchModule_isLoaded(ExecuTorchModule_Handle module);

// Method management
bool ExecuTorchModule_loadMethod(ExecuTorchModule_Handle module, const char* methodName, char** error);
bool ExecuTorchModule_isMethodLoaded(ExecuTorchModule_Handle module, const char* methodName);
char** ExecuTorchModule_getMethodNames(ExecuTorchModule_Handle module, size_t* count, char** error);
void ExecuTorchModule_freeMethodNames(char** methodNames, size_t count);

// Method execution
typedef void* ExecuTorchValue_Handle;
ExecuTorchValue_Handle* ExecuTorchModule_executeMethod(ExecuTorchModule_Handle module, const char* methodName, 
                                                       ExecuTorchValue_Handle* inputs, size_t inputCount, 
                                                       size_t* outputCount, char** error);
void ExecuTorchModule_freeOutputs(ExecuTorchValue_Handle* outputs, size_t count);

#ifdef __cplusplus
}
#endif
EOF
            cat >> "$BRIDGE_DIR/${header}_Bridge.mm" << EOF
// Bridge implementation for ExecuTorchModule

ExecuTorchModule_Handle ExecuTorchModule_create(const char* filePath, ExecutorchModuleLoadMode_t loadMode) {
    NSString* path = [NSString stringWithUTF8String:filePath];
    ExecuTorchModuleLoadMode mode = (ExecuTorchModuleLoadMode)loadMode;
    ExecuTorchModule* module = [[ExecuTorchModule alloc] initWithFilePath:path loadMode:mode];
    return (__bridge_retained void*)module;
}

ExecuTorchModule_Handle ExecuTorchModule_createWithDefaultMode(const char* filePath) {
    NSString* path = [NSString stringWithUTF8String:filePath];
    ExecuTorchModule* module = [[ExecuTorchModule alloc] initWithFilePath:path];
    return (__bridge_retained void*)module;
}

void ExecuTorchModule_destroy(ExecuTorchModule_Handle module) {
    if (module) {
        CFBridgingRelease(module);
    }
}

bool ExecuTorchModule_loadWithVerification(ExecuTorchModule_Handle module, ExecutorchVerification_t verification, char** error) {
    ExecuTorchModule* objcModule = (__bridge ExecuTorchModule*)module;
    NSError* nsError = nil;
    ExecuTorchVerification objcVerification = (ExecuTorchVerification)verification;
    BOOL success = [objcModule loadWithVerification:objcVerification error:&nsError];
    
    if (!success && error && nsError) {
        NSString* errorString = nsError.localizedDescription;
        *error = strdup([errorString UTF8String]);
    }
    
    return success;
}

bool ExecuTorchModule_load(ExecuTorchModule_Handle module, char** error) {
    ExecuTorchModule* objcModule = (__bridge ExecuTorchModule*)module;
    NSError* nsError = nil;
    BOOL success = [objcModule load:&nsError];
    
    if (!success && error && nsError) {
        NSString* errorString = nsError.localizedDescription;
        *error = strdup([errorString UTF8String]);
    }
    
    return success;
}

bool ExecuTorchModule_isLoaded(ExecuTorchModule_Handle module) {
    ExecuTorchModule* objcModule = (__bridge ExecuTorchModule*)module;
    return [objcModule isLoaded];
}

bool ExecuTorchModule_loadMethod(ExecuTorchModule_Handle module, const char* methodName, char** error) {
    ExecuTorchModule* objcModule = (__bridge ExecuTorchModule*)module;
    NSString* method = [NSString stringWithUTF8String:methodName];
    NSError* nsError = nil;
    BOOL success = [objcModule loadMethod:method error:&nsError];
    
    if (!success && error && nsError) {
        NSString* errorString = nsError.localizedDescription;
        *error = strdup([errorString UTF8String]);
    }
    
    return success;
}

bool ExecuTorchModule_isMethodLoaded(ExecuTorchModule_Handle module, const char* methodName) {
    ExecuTorchModule* objcModule = (__bridge ExecuTorchModule*)module;
    NSString* method = [NSString stringWithUTF8String:methodName];
    return [objcModule isMethodLoaded:method];
}

char** ExecuTorchModule_getMethodNames(ExecuTorchModule_Handle module, size_t* count, char** error) {
    ExecuTorchModule* objcModule = (__bridge ExecuTorchModule*)module;
    NSError* nsError = nil;
    NSSet<NSString*>* methodNames = [objcModule methodNames:&nsError];
    
    if (!methodNames) {
        if (error && nsError) {
            NSString* errorString = nsError.localizedDescription;
            *error = strdup([errorString UTF8String]);
        }
        *count = 0;
        return nullptr;
    }
    
    *count = methodNames.count;
    char** result = (char**)malloc(sizeof(char*) * methodNames.count);
    size_t index = 0;
    for (NSString* name in methodNames) {
        result[index++] = strdup([name UTF8String]);
    }
    
    return result;
}

void ExecuTorchModule_freeMethodNames(char** methodNames, size_t count) {
    if (methodNames) {
        for (size_t i = 0; i < count; i++) {
            free(methodNames[i]);
        }
        free(methodNames);
    }
}

ExecuTorchValue_Handle* ExecuTorchModule_executeMethod(ExecuTorchModule_Handle module, const char* methodName, 
                                                       ExecuTorchValue_Handle* inputs, size_t inputCount, 
                                                       size_t* outputCount, char** error) {
    ExecuTorchModule* objcModule = (__bridge ExecuTorchModule*)module;
    NSString* method = [NSString stringWithUTF8String:methodName];
    
    // Convert C inputs to Objective-C array
    NSMutableArray<ExecuTorchValue*>* objcInputs = [[NSMutableArray alloc] init];
    for (size_t i = 0; i < inputCount; i++) {
        ExecuTorchValue* value = (__bridge ExecuTorchValue*)inputs[i];
        [objcInputs addObject:value];
    }
    
    NSError* nsError = nil;
    NSArray<ExecuTorchValue*>* outputs = [objcModule executeMethod:method withInputs:objcInputs error:&nsError];
    
    if (!outputs) {
        if (error && nsError) {
            NSString* errorString = nsError.localizedDescription;
            *error = strdup([errorString UTF8String]);
        }
        *outputCount = 0;
        return nullptr;
    }
    
    *outputCount = outputs.count;
    ExecuTorchValue_Handle* result = (ExecuTorchValue_Handle*)malloc(sizeof(ExecuTorchValue_Handle) * outputs.count);
    for (size_t i = 0; i < outputs.count; i++) {
        result[i] = (__bridge_retained void*)outputs[i];
    }
    
    return result;
}

void ExecuTorchModule_freeOutputs(ExecuTorchValue_Handle* outputs, size_t count) {
    if (outputs) {
        for (size_t i = 0; i < count; i++) {
            if (outputs[i]) {
                CFBridgingRelease(outputs[i]);
            }
        }
        free(outputs);
    }
}

#ifdef __cplusplus
}
#endif
EOF
            ;;
        
        "ExecuTorchTensor")
            cat >> "$BRIDGE_DIR/${header}_Bridge.h" << EOF
// Tensor data types
typedef enum {
    ET_DATA_TYPE_BYTE = 0,
    ET_DATA_TYPE_CHAR,
    ET_DATA_TYPE_SHORT,
    ET_DATA_TYPE_INT,
    ET_DATA_TYPE_LONG,
    ET_DATA_TYPE_HALF,
    ET_DATA_TYPE_FLOAT,
    ET_DATA_TYPE_DOUBLE,
    ET_DATA_TYPE_COMPLEX_HALF,
    ET_DATA_TYPE_COMPLEX_FLOAT,
    ET_DATA_TYPE_COMPLEX_DOUBLE,
    ET_DATA_TYPE_BOOL,
    ET_DATA_TYPE_QINT8,
    ET_DATA_TYPE_QUINT8,
    ET_DATA_TYPE_QINT32,
    ET_DATA_TYPE_BFLOAT16,
    ET_DATA_TYPE_QUINT4X2,
    ET_DATA_TYPE_QUINT2X4,
    ET_DATA_TYPE_BITS1X8,
    ET_DATA_TYPE_BITS2X4,
    ET_DATA_TYPE_BITS4X2,
    ET_DATA_TYPE_BITS8,
    ET_DATA_TYPE_BITS16,
    ET_DATA_TYPE_FLOAT8_E5M2,
    ET_DATA_TYPE_FLOAT8_E4M3FN,
    ET_DATA_TYPE_FLOAT8_E5M2FNUZ,
    ET_DATA_TYPE_FLOAT8_E4M3FNUZ,
    ET_DATA_TYPE_UINT16,
    ET_DATA_TYPE_UINT32,
    ET_DATA_TYPE_UINT64,
    ET_DATA_TYPE_UNDEFINED,
    ET_DATA_TYPE_NUM_OPTIONS
} ExecutorchDataType_t;

// Shape dynamism
typedef enum {
    ET_SHAPE_DYNAMISM_STATIC = 0,
    ET_SHAPE_DYNAMISM_DYNAMIC_BOUND,
    ET_SHAPE_DYNAMISM_DYNAMIC_UNBOUND
} ExecutorchShapeDynamism_t;

// Utility functions
size_t ExecuTorchTensor_sizeOfDataType(ExecutorchDataType_t dataType);
size_t ExecuTorchTensor_elementCountOfShape(const int64_t* shape, size_t shapeDims);

// Tensor creation and management
ExecuTorchTensor_Handle ExecuTorchTensor_create(void* data, const int64_t* shape, size_t shapeDims, 
                                                const int64_t* strides, size_t strideDims,
                                                const int64_t* dimensionOrder, size_t dimensionOrderDims,
                                                ExecutorchDataType_t dataType, ExecutorchShapeDynamism_t shapeDynamism);
ExecuTorchTensor_Handle ExecuTorchTensor_createSimple(void* data, const int64_t* shape, size_t shapeDims, ExecutorchDataType_t dataType);
ExecuTorchTensor_Handle ExecuTorchTensor_copy(ExecuTorchTensor_Handle tensor);
void ExecuTorchTensor_destroy(ExecuTorchTensor_Handle tensor);

// Scalar tensor creation
ExecuTorchTensor_Handle ExecuTorchTensor_createFromFloat(float scalar);
ExecuTorchTensor_Handle ExecuTorchTensor_createFromDouble(double scalar);
ExecuTorchTensor_Handle ExecuTorchTensor_createFromInt32(int32_t scalar);
ExecuTorchTensor_Handle ExecuTorchTensor_createFromInt64(int64_t scalar);
ExecuTorchTensor_Handle ExecuTorchTensor_createFromBool(bool scalar);

// Tensor properties
ExecutorchDataType_t ExecuTorchTensor_getDataType(ExecuTorchTensor_Handle tensor);
const int64_t* ExecuTorchTensor_getShape(ExecuTorchTensor_Handle tensor, size_t* dims);
const int64_t* ExecuTorchTensor_getStrides(ExecuTorchTensor_Handle tensor, size_t* dims);
const int64_t* ExecuTorchTensor_getDimensionOrder(ExecuTorchTensor_Handle tensor, size_t* dims);
ExecutorchShapeDynamism_t ExecuTorchTensor_getShapeDynamism(ExecuTorchTensor_Handle tensor);
size_t ExecuTorchTensor_getElementCount(ExecuTorchTensor_Handle tensor);

// Data access
void ExecuTorchTensor_getData(ExecuTorchTensor_Handle tensor, void** data, size_t* elementCount, ExecutorchDataType_t* dataType);
void ExecuTorchTensor_getMutableData(ExecuTorchTensor_Handle tensor, void** data, size_t* elementCount, ExecutorchDataType_t* dataType);

// Tensor operations
bool ExecuTorchTensor_resizeToShape(ExecuTorchTensor_Handle tensor, const int64_t* shape, size_t shapeDims, char** error);

#ifdef __cplusplus
}
#endif
EOF
            cat >> "$BRIDGE_DIR/${header}_Bridge.mm" << EOF
// Bridge implementation for ExecuTorchTensor

size_t ExecuTorchTensor_sizeOfDataType(ExecutorchDataType_t dataType) {
    return ExecuTorchSizeOfDataType((ExecuTorchDataType)dataType);
}

size_t ExecuTorchTensor_elementCountOfShape(const int64_t* shape, size_t shapeDims) {
    NSMutableArray<NSNumber*>* shapeArray = [[NSMutableArray alloc] init];
    for (size_t i = 0; i < shapeDims; i++) {
        [shapeArray addObject:@(shape[i])];
    }
    return ExecuTorchElementCountOfShape(shapeArray);
}

ExecuTorchTensor_Handle ExecuTorchTensor_create(void* data, const int64_t* shape, size_t shapeDims, 
                                                const int64_t* strides, size_t strideDims,
                                                const int64_t* dimensionOrder, size_t dimensionOrderDims,
                                                ExecutorchDataType_t dataType, ExecutorchShapeDynamism_t shapeDynamism) {
    NSData* nsData = [NSData dataWithBytes:data length:ExecuTorchTensor_sizeOfDataType(dataType) * ExecuTorchTensor_elementCountOfShape(shape, shapeDims)];
    
    NSMutableArray<NSNumber*>* shapeArray = [[NSMutableArray alloc] init];
    for (size_t i = 0; i < shapeDims; i++) {
        [shapeArray addObject:@(shape[i])];
    }
    
    NSMutableArray<NSNumber*>* stridesArray = [[NSMutableArray alloc] init];
    for (size_t i = 0; i < strideDims; i++) {
        [stridesArray addObject:@(strides[i])];
    }
    
    NSMutableArray<NSNumber*>* dimensionOrderArray = [[NSMutableArray alloc] init];
    for (size_t i = 0; i < dimensionOrderDims; i++) {
        [dimensionOrderArray addObject:@(dimensionOrder[i])];
    }
    
    ExecuTorchTensor* tensor = [[ExecuTorchTensor alloc] initWithData:nsData
                                                                shape:shapeArray
                                                              strides:stridesArray
                                                       dimensionOrder:dimensionOrderArray
                                                             dataType:(ExecuTorchDataType)dataType
                                                        shapeDynamism:(ExecuTorchShapeDynamism)shapeDynamism];
    return (__bridge_retained void*)tensor;
}

ExecuTorchTensor_Handle ExecuTorchTensor_createSimple(void* data, const int64_t* shape, size_t shapeDims, ExecutorchDataType_t dataType) {
    NSData* nsData = [NSData dataWithBytes:data length:ExecuTorchTensor_sizeOfDataType(dataType) * ExecuTorchTensor_elementCountOfShape(shape, shapeDims)];
    
    NSMutableArray<NSNumber*>* shapeArray = [[NSMutableArray alloc] init];
    for (size_t i = 0; i < shapeDims; i++) {
        [shapeArray addObject:@(shape[i])];
    }
    
    ExecuTorchTensor* tensor = [[ExecuTorchTensor alloc] initWithData:nsData
                                                                shape:shapeArray
                                                             dataType:(ExecuTorchDataType)dataType];
    return (__bridge_retained void*)tensor;
}

ExecuTorchTensor_Handle ExecuTorchTensor_copy(ExecuTorchTensor_Handle tensor) {
    ExecuTorchTensor* objcTensor = (__bridge ExecuTorchTensor*)tensor;
    ExecuTorchTensor* copy = [objcTensor copy];
    return (__bridge_retained void*)copy;
}

void ExecuTorchTensor_destroy(ExecuTorchTensor_Handle tensor) {
    if (tensor) {
        CFBridgingRelease(tensor);
    }
}

ExecuTorchTensor_Handle ExecuTorchTensor_createFromFloat(float scalar) {
    ExecuTorchTensor* tensor = [[ExecuTorchTensor alloc] initWithFloat:scalar];
    return (__bridge_retained void*)tensor;
}

ExecuTorchTensor_Handle ExecuTorchTensor_createFromDouble(double scalar) {
    ExecuTorchTensor* tensor = [[ExecuTorchTensor alloc] initWithDouble:scalar];
    return (__bridge_retained void*)tensor;
}

ExecuTorchTensor_Handle ExecuTorchTensor_createFromInt32(int32_t scalar) {
    ExecuTorchTensor* tensor = [[ExecuTorchTensor alloc] initWithInt:scalar];
    return (__bridge_retained void*)tensor;
}

ExecuTorchTensor_Handle ExecuTorchTensor_createFromInt64(int64_t scalar) {
    ExecuTorchTensor* tensor = [[ExecuTorchTensor alloc] initWithLong:scalar];
    return (__bridge_retained void*)tensor;
}

ExecuTorchTensor_Handle ExecuTorchTensor_createFromBool(bool scalar) {
    ExecuTorchTensor* tensor = [[ExecuTorchTensor alloc] initWithBool:scalar];
    return (__bridge_retained void*)tensor;
}

ExecutorchDataType_t ExecuTorchTensor_getDataType(ExecuTorchTensor_Handle tensor) {
    ExecuTorchTensor* objcTensor = (__bridge ExecuTorchTensor*)tensor;
    return (ExecutorchDataType_t)objcTensor.dataType;
}

// Global storage for shape arrays to ensure they remain valid
static NSMutableDictionary<NSValue*, NSArray<NSNumber*>*>* shapeStorage = nil;

const int64_t* ExecuTorchTensor_getShape(ExecuTorchTensor_Handle tensor, size_t* dims) {
    ExecuTorchTensor* objcTensor = (__bridge ExecuTorchTensor*)tensor;
    NSArray<NSNumber*>* shape = objcTensor.shape;
    
    if (!shapeStorage) {
        shapeStorage = [[NSMutableDictionary alloc] init];
    }
    
    NSValue* key = [NSValue valueWithPointer:tensor];
    shapeStorage[key] = shape;
    
    *dims = shape.count;
    
    static int64_t* shapeBuffer = nullptr;
    static size_t bufferSize = 0;
    
    if (bufferSize < shape.count) {
        free(shapeBuffer);
        bufferSize = shape.count;
        shapeBuffer = (int64_t*)malloc(sizeof(int64_t) * bufferSize);
    }
    
    for (size_t i = 0; i < shape.count; i++) {
        shapeBuffer[i] = shape[i].longLongValue;
    }
    
    return shapeBuffer;
}

const int64_t* ExecuTorchTensor_getStrides(ExecuTorchTensor_Handle tensor, size_t* dims) {
    ExecuTorchTensor* objcTensor = (__bridge ExecuTorchTensor*)tensor;
    NSArray<NSNumber*>* strides = objcTensor.strides;
    
    *dims = strides.count;
    
    static int64_t* stridesBuffer = nullptr;
    static size_t bufferSize = 0;
    
    if (bufferSize < strides.count) {
        free(stridesBuffer);
        bufferSize = strides.count;
        stridesBuffer = (int64_t*)malloc(sizeof(int64_t) * bufferSize);
    }
    
    for (size_t i = 0; i < strides.count; i++) {
        stridesBuffer[i] = strides[i].longLongValue;
    }
    
    return stridesBuffer;
}

const int64_t* ExecuTorchTensor_getDimensionOrder(ExecuTorchTensor_Handle tensor, size_t* dims) {
    ExecuTorchTensor* objcTensor = (__bridge ExecuTorchTensor*)tensor;
    NSArray<NSNumber*>* dimensionOrder = objcTensor.dimensionOrder;
    
    *dims = dimensionOrder.count;
    
    static int64_t* dimensionOrderBuffer = nullptr;
    static size_t bufferSize = 0;
    
    if (bufferSize < dimensionOrder.count) {
        free(dimensionOrderBuffer);
        bufferSize = dimensionOrder.count;
        dimensionOrderBuffer = (int64_t*)malloc(sizeof(int64_t) * bufferSize);
    }
    
    for (size_t i = 0; i < dimensionOrder.count; i++) {
        dimensionOrderBuffer[i] = dimensionOrder[i].longLongValue;
    }
    
    return dimensionOrderBuffer;
}

ExecutorchShapeDynamism_t ExecuTorchTensor_getShapeDynamism(ExecuTorchTensor_Handle tensor) {
    ExecuTorchTensor* objcTensor = (__bridge ExecuTorchTensor*)tensor;
    return (ExecutorchShapeDynamism_t)objcTensor.shapeDynamism;
}

size_t ExecuTorchTensor_getElementCount(ExecuTorchTensor_Handle tensor) {
    ExecuTorchTensor* objcTensor = (__bridge ExecuTorchTensor*)tensor;
    return objcTensor.count;
}

void ExecuTorchTensor_getData(ExecuTorchTensor_Handle tensor, void** data, size_t* elementCount, ExecutorchDataType_t* dataType) {
    ExecuTorchTensor* objcTensor = (__bridge ExecuTorchTensor*)tensor;
    [objcTensor bytesWithHandler:^(const void *pointer, NSInteger count, ExecuTorchDataType dt) {
        *data = (void*)pointer;
        *elementCount = count;
        *dataType = (ExecutorchDataType_t)dt;
    }];
}

void ExecuTorchTensor_getMutableData(ExecuTorchTensor_Handle tensor, void** data, size_t* elementCount, ExecutorchDataType_t* dataType) {
    ExecuTorchTensor* objcTensor = (__bridge ExecuTorchTensor*)tensor;
    [objcTensor mutableBytesWithHandler:^(void *pointer, NSInteger count, ExecuTorchDataType dt) {
        *data = pointer;
        *elementCount = count;
        *dataType = (ExecutorchDataType_t)dt;
    }];
}

bool ExecuTorchTensor_resizeToShape(ExecuTorchTensor_Handle tensor, const int64_t* shape, size_t shapeDims, char** error) {
    ExecuTorchTensor* objcTensor = (__bridge ExecuTorchTensor*)tensor;
    
    NSMutableArray<NSNumber*>* shapeArray = [[NSMutableArray alloc] init];
    for (size_t i = 0; i < shapeDims; i++) {
        [shapeArray addObject:@(shape[i])];
    }
    
    NSError* nsError = nil;
    BOOL success = [objcTensor resizeToShape:shapeArray error:&nsError];
    
    if (!success && error && nsError) {
        NSString* errorString = nsError.localizedDescription;
        *error = strdup([errorString UTF8String]);
    }
    
    return success;
}

#ifdef __cplusplus
}
#endif
EOF
            ;;
        
        "ExecuTorchValue")
            # Add cross-dependency include for ExecuTorchTensor_Handle
            cat >> "$BRIDGE_DIR/${header}_Bridge.h" << EOF
#include "ExecuTorchTensor_Bridge.h"

// Value tags
typedef enum {
    ET_VALUE_TAG_NONE = 0,
    ET_VALUE_TAG_TENSOR,
    ET_VALUE_TAG_STRING,
    ET_VALUE_TAG_DOUBLE,
    ET_VALUE_TAG_INTEGER,
    ET_VALUE_TAG_BOOLEAN,
    ET_VALUE_TAG_BOOLEAN_LIST,
    ET_VALUE_TAG_DOUBLE_LIST,
    ET_VALUE_TAG_INTEGER_LIST,
    ET_VALUE_TAG_TENSOR_LIST,
    ET_VALUE_TAG_SCALAR_LIST,
    ET_VALUE_TAG_OPTIONAL_TENSOR_LIST
} ExecutorchValueTag_t;

// Value creation
ExecuTorchValue_Handle ExecuTorchValue_createWithTensor(ExecuTorchTensor_Handle tensor);
ExecuTorchValue_Handle ExecuTorchValue_createWithString(const char* string);
ExecuTorchValue_Handle ExecuTorchValue_createWithBoolean(bool value);
ExecuTorchValue_Handle ExecuTorchValue_createWithInteger(int64_t value);
ExecuTorchValue_Handle ExecuTorchValue_createWithDouble(double value);
void ExecuTorchValue_destroy(ExecuTorchValue_Handle value);

// Value properties
ExecutorchValueTag_t ExecuTorchValue_getTag(ExecuTorchValue_Handle value);
bool ExecuTorchValue_isNone(ExecuTorchValue_Handle value);
bool ExecuTorchValue_isTensor(ExecuTorchValue_Handle value);
bool ExecuTorchValue_isString(ExecuTorchValue_Handle value);
bool ExecuTorchValue_isScalar(ExecuTorchValue_Handle value);
bool ExecuTorchValue_isBoolean(ExecuTorchValue_Handle value);
bool ExecuTorchValue_isInteger(ExecuTorchValue_Handle value);
bool ExecuTorchValue_isDouble(ExecuTorchValue_Handle value);

// Value extraction
ExecuTorchTensor_Handle ExecuTorchValue_getTensor(ExecuTorchValue_Handle value);
const char* ExecuTorchValue_getString(ExecuTorchValue_Handle value);
bool ExecuTorchValue_getBoolean(ExecuTorchValue_Handle value);
int64_t ExecuTorchValue_getInteger(ExecuTorchValue_Handle value);
double ExecuTorchValue_getDouble(ExecuTorchValue_Handle value);

// Value comparison
bool ExecuTorchValue_isEqual(ExecuTorchValue_Handle value1, ExecuTorchValue_Handle value2);

#ifdef __cplusplus
}
#endif
EOF
            cat >> "$BRIDGE_DIR/${header}_Bridge.mm" << EOF
// Bridge implementation for ExecuTorchValue

ExecuTorchValue_Handle ExecuTorchValue_createWithTensor(ExecuTorchTensor_Handle tensor) {
    ExecuTorchTensor* objcTensor = (__bridge ExecuTorchTensor*)tensor;
    ExecuTorchValue* value = [ExecuTorchValue valueWithTensor:objcTensor];
    return (__bridge_retained void*)value;
}

ExecuTorchValue_Handle ExecuTorchValue_createWithString(const char* string) {
    NSString* objcString = [NSString stringWithUTF8String:string];
    ExecuTorchValue* value = [ExecuTorchValue valueWithString:objcString];
    return (__bridge_retained void*)value;
}

ExecuTorchValue_Handle ExecuTorchValue_createWithBoolean(bool value) {
    ExecuTorchValue* objcValue = [ExecuTorchValue valueWithBoolean:value];
    return (__bridge_retained void*)objcValue;
}

ExecuTorchValue_Handle ExecuTorchValue_createWithInteger(int64_t value) {
    ExecuTorchValue* objcValue = [ExecuTorchValue valueWithInteger:value];
    return (__bridge_retained void*)objcValue;
}

ExecuTorchValue_Handle ExecuTorchValue_createWithDouble(double value) {
    ExecuTorchValue* objcValue = [ExecuTorchValue valueWithDouble:value];
    return (__bridge_retained void*)objcValue;
}

void ExecuTorchValue_destroy(ExecuTorchValue_Handle value) {
    if (value) {
        CFBridgingRelease(value);
    }
}

ExecutorchValueTag_t ExecuTorchValue_getTag(ExecuTorchValue_Handle value) {
    ExecuTorchValue* objcValue = (__bridge ExecuTorchValue*)value;
    return (ExecutorchValueTag_t)objcValue.tag;
}

bool ExecuTorchValue_isNone(ExecuTorchValue_Handle value) {
    ExecuTorchValue* objcValue = (__bridge ExecuTorchValue*)value;
    return objcValue.isNone;
}

bool ExecuTorchValue_isTensor(ExecuTorchValue_Handle value) {
    ExecuTorchValue* objcValue = (__bridge ExecuTorchValue*)value;
    return objcValue.isTensor;
}

bool ExecuTorchValue_isString(ExecuTorchValue_Handle value) {
    ExecuTorchValue* objcValue = (__bridge ExecuTorchValue*)value;
    return objcValue.isString;
}

bool ExecuTorchValue_isScalar(ExecuTorchValue_Handle value) {
    ExecuTorchValue* objcValue = (__bridge ExecuTorchValue*)value;
    return objcValue.isScalar;
}

bool ExecuTorchValue_isBoolean(ExecuTorchValue_Handle value) {
    ExecuTorchValue* objcValue = (__bridge ExecuTorchValue*)value;
    return objcValue.isBoolean;
}

bool ExecuTorchValue_isInteger(ExecuTorchValue_Handle value) {
    ExecuTorchValue* objcValue = (__bridge ExecuTorchValue*)value;
    return objcValue.isInteger;
}

bool ExecuTorchValue_isDouble(ExecuTorchValue_Handle value) {
    ExecuTorchValue* objcValue = (__bridge ExecuTorchValue*)value;
    return objcValue.isDouble;
}

ExecuTorchTensor_Handle ExecuTorchValue_getTensor(ExecuTorchValue_Handle value) {
    ExecuTorchValue* objcValue = (__bridge ExecuTorchValue*)value;
    ExecuTorchTensor* tensor = objcValue.tensorValue;
    return tensor ? (__bridge void*)tensor : nullptr;
}

const char* ExecuTorchValue_getString(ExecuTorchValue_Handle value) {
    ExecuTorchValue* objcValue = (__bridge ExecuTorchValue*)value;
    NSString* string = objcValue.stringValue;
    return string ? [string UTF8String] : nullptr;
}

bool ExecuTorchValue_getBoolean(ExecuTorchValue_Handle value) {
    ExecuTorchValue* objcValue = (__bridge ExecuTorchValue*)value;
    return objcValue.boolValue;
}

int64_t ExecuTorchValue_getInteger(ExecuTorchValue_Handle value) {
    ExecuTorchValue* objcValue = (__bridge ExecuTorchValue*)value;
    return objcValue.intValue;
}

double ExecuTorchValue_getDouble(ExecuTorchValue_Handle value) {
    ExecuTorchValue* objcValue = (__bridge ExecuTorchValue*)value;
    return objcValue.doubleValue;
}

bool ExecuTorchValue_isEqual(ExecuTorchValue_Handle value1, ExecuTorchValue_Handle value2) {
    ExecuTorchValue* objcValue1 = (__bridge ExecuTorchValue*)value1;
    ExecuTorchValue* objcValue2 = (__bridge ExecuTorchValue*)value2;
    return [objcValue1 isEqualToValue:objcValue2];
}

#ifdef __cplusplus
}
#endif
EOF
            ;;
    esac
    
    echo "Generated bridge for $header"
done

# Create a combined header for convenience
cat > "$BRIDGE_DIR/ExecutTorch_Bridge.h" << 'EOF'
/*
 * Combined C++ Bridge for ExecutTorch
 * Auto-generated - DO NOT EDIT MANUALLY
 * 
 * This file includes all ExecutTorch C++ bridge headers for convenience
 */

#pragma once

#include "ExecuTorch_Bridge.h"
#include "ExecuTorchError_Bridge.h"
#include "ExecuTorchLog_Bridge.h"
#include "ExecuTorchModule_Bridge.h"
#include "ExecuTorchTensor_Bridge.h"
#include "ExecuTorchValue_Bridge.h"

// Convenience macros for error handling
#define ET_CHECK_ERROR(error) \
    do { \
        if (error) { \
            fprintf(stderr, "ExecutTorch Error: %s\n", error); \
            free(error); \
            return false; \
        } \
    } while(0)

#define ET_CHECK_ERROR_GOTO(error, label) \
    do { \
        if (error) { \
            fprintf(stderr, "ExecutTorch Error: %s\n", error); \
            free(error); \
            goto label; \
        } \
    } while(0)
EOF

echo "Bridge generation completed!"
echo "Generated files in: $BRIDGE_DIR"
ls -la "$BRIDGE_DIR" 