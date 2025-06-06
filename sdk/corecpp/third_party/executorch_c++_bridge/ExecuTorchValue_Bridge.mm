/*
 * C++ Bridge Implementation for ExecutTorch ExecuTorchValue
 * Auto-generated - DO NOT EDIT MANUALLY
 */

#import <Foundation/Foundation.h>
#import <executorch/ExecuTorchValue.h>
#include "ExecuTorchValue_Bridge.h"

#ifdef __cplusplus
extern "C" {
#endif

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
