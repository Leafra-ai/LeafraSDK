/*
 * C++ Bridge for ExecutTorch ExecuTorchValue
 * Auto-generated - DO NOT EDIT MANUALLY
 * 
 * This file provides a C++ interface to the ExecutTorch ExecuTorchValue Objective-C API
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
typedef void* ExecuTorchValue_Handle;

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
