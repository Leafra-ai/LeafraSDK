/*
 * C++ Bridge Implementation for ExecutTorch ExecuTorchTensor
 * Auto-generated - DO NOT EDIT MANUALLY
 */

#import <Foundation/Foundation.h>
#import <executorch/ExecuTorchTensor.h>
#include "ExecuTorchTensor_Bridge.h"

#ifdef __cplusplus
extern "C" {
#endif

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
