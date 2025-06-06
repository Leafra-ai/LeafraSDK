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
