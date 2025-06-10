#import <Foundation/Foundation.h>
#import <CoreML/CoreML.h>
#include "leafra/leafra_coreml.h"
#include "leafra/logger.h"

namespace leafra {

struct CoreMLModelImpl {
    MLModel* __strong model;
    MLPredictionOptions* __strong predictionOptions;
    NSString* __strong modelPath;
    NSString* __strong computeUnits;
};

// Create CoreML model instance
void* coreml_create_model(const char* model_path, const char* compute_units) {
    @autoreleasepool {
        NSString* nsModelPath = [NSString stringWithUTF8String:model_path];
        NSString* nsComputeUnits = [NSString stringWithUTF8String:compute_units];
        
        // Check if file exists
        NSFileManager* fileManager = [NSFileManager defaultManager];
        if (![fileManager fileExistsAtPath:nsModelPath]) {
            LEAFRA_ERROR() << "CoreML model file not found: " << model_path;
            return nullptr;
        }
        
        NSURL* modelURL = [NSURL fileURLWithPath:nsModelPath];
        NSError* error = nil;
        
        // Configure model loading options
        MLModelConfiguration* config = [[MLModelConfiguration alloc] init];
        
        // Set compute units based on configuration
        if ([nsComputeUnits isEqualToString:@"cpuOnly"]) {
            config.computeUnits = MLComputeUnitsCPUOnly;
        } else if ([nsComputeUnits isEqualToString:@"cpuAndGPU"]) {
            config.computeUnits = MLComputeUnitsCPUAndGPU;
        } else if ([nsComputeUnits isEqualToString:@"cpuAndNeuralEngine"]) {
            if (@available(iOS 13.0, macOS 10.15, *)) {
                config.computeUnits = MLComputeUnitsCPUAndNeuralEngine;
            } else {
                config.computeUnits = MLComputeUnitsCPUAndGPU;
                LEAFRA_WARNING() << "Neural Engine not available on this OS version, falling back to CPU+GPU";
            }
        } else {
            // Default: all available compute units
            config.computeUnits = MLComputeUnitsAll;
        }
        
        // Load the model
        MLModel* model = [MLModel modelWithContentsOfURL:modelURL configuration:config error:&error];
        if (!model || error) {
            NSString* errorDescription = error ? error.localizedDescription : @"Unknown error";
            LEAFRA_ERROR() << "Failed to load CoreML model: " << [errorDescription UTF8String];
            return nullptr;
        }
        
        // Create prediction options
        MLPredictionOptions* predictionOptions = [[MLPredictionOptions alloc] init];
        // Note: usesCPUOnly is deprecated, compute units are now configured via MLModelConfiguration
        
        // Create wrapper struct
        CoreMLModelImpl* impl = new CoreMLModelImpl();
        impl->model = model;
        impl->predictionOptions = predictionOptions;
        impl->modelPath = nsModelPath;
        impl->computeUnits = nsComputeUnits;
        
        LEAFRA_INFO() << "✅ CoreML model loaded successfully";
        LEAFRA_INFO() << "  - Path: " << model_path;
        LEAFRA_INFO() << "  - Compute units: " << compute_units;
        
        return impl;
    }
}

// Get model description
bool coreml_get_model_description(void* model_ptr, char* description, size_t max_length) {
    if (!model_ptr) return false;
    
    @autoreleasepool {
        CoreMLModelImpl* impl = static_cast<CoreMLModelImpl*>(model_ptr);
        MLModel* model = impl->model;
        
        if (!model) return false;
        
        MLModelDescription* modelDescription = model.modelDescription;
        NSString* descStr = [NSString stringWithFormat:@"Model: %@\nInputs: %lu\nOutputs: %lu", 
                            @"CoreML Model", // Simplified since MLModelMetadataKeyDescription may not be available
                            (unsigned long)modelDescription.inputDescriptionsByName.count,
                            (unsigned long)modelDescription.outputDescriptionsByName.count];
        
        const char* cStr = [descStr UTF8String];
        size_t length = strlen(cStr);
        if (length >= max_length) {
            length = max_length - 1;
        }
        
        strncpy(description, cStr, length);
        description[length] = '\0';
        
        return true;
    }
}

// Perform prediction (placeholder - will be expanded for specific use cases)
bool coreml_predict(void* model_ptr, const float* input_data, size_t input_size, 
                   float* output_data, size_t output_size) {
    if (!model_ptr || !input_data || !output_data) return false;
    
    @autoreleasepool {
        CoreMLModelImpl* impl = static_cast<CoreMLModelImpl*>(model_ptr);
        MLModel* model = impl->model;
        
        if (!model) return false;
        
        // This is a placeholder implementation
        // Real implementation would depend on the specific model's input/output format
        LEAFRA_INFO() << "CoreML prediction called (placeholder implementation)";
        LEAFRA_INFO() << "  - Input size: " << input_size;
        LEAFRA_INFO() << "  - Output size: " << output_size;
        
        // For now, just zero the output
        memset(output_data, 0, output_size * sizeof(float));
        
        return true;
    }
}

// Destroy CoreML model
void coreml_destroy_model(void* model_ptr) {
    if (!model_ptr) return;
    
    @autoreleasepool {
        CoreMLModelImpl* impl = static_cast<CoreMLModelImpl*>(model_ptr);
        
        // ARC will automatically release the Objective-C objects
        impl->model = nil;
        impl->predictionOptions = nil;
        impl->modelPath = nil;
        impl->computeUnits = nil;
        
        delete impl;
    }
}

} // namespace leafra 