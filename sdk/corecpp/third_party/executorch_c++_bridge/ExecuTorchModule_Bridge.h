/*
 * C++ Bridge for ExecutTorch ExecuTorchModule
 * Auto-generated - DO NOT EDIT MANUALLY
 * 
 * This file provides a C++ interface to the ExecutTorch ExecuTorchModule Objective-C API
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
typedef void* ExecuTorchModule_Handle;

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
