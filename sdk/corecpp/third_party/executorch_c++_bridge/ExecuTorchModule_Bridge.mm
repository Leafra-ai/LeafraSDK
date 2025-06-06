/*
 * C++ Bridge Implementation for ExecutTorch ExecuTorchModule
 * Auto-generated - DO NOT EDIT MANUALLY
 */

#import <Foundation/Foundation.h>
#import <executorch/ExecuTorchModule.h>
#include "ExecuTorchModule_Bridge.h"

#ifdef __cplusplus
extern "C" {
#endif

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
