/*
 * C++ Bridge for ExecutTorch ExecuTorchTensor
 * Auto-generated - DO NOT EDIT MANUALLY
 * 
 * This file provides a C++ interface to the ExecutTorch ExecuTorchTensor Objective-C API
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
typedef void* ExecuTorchTensor_Handle;

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
