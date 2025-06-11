#import <Foundation/Foundation.h>
#import <CoreML/CoreML.h>
#include "leafra/leafra_coreml.h"
#include "leafra/logger.h"
#include <stdexcept>
#include <string>

namespace leafra {

// Internal implementation struct
struct CoreMLModelImpl {
    MLModel* model;
    MLPredictionOptions* predictionOptions;
    NSString* modelPath;
    NSString* computeUnits;
};

} // namespace leafra

// C++ Class Implementation
namespace leafra {

// Static helper function
const char* CoreMLModel::computeUnitsToString(ComputeUnits units) {
    switch (units) {
        case ComputeUnits::CPUOnly: return "cpu";
        case ComputeUnits::All: return "all";
        case ComputeUnits::CPUAndGPU: return "cpu_and_gpu";
        case ComputeUnits::CPUAndNeuralEngine: return "cpu_and_neural_engine";
        default: return "all";
    }
}

// Constructor
CoreMLModel::CoreMLModel(const std::string& model_path, ComputeUnits compute_units)
    : model_ptr_(nullptr), cached_input_nsnames_(nullptr), cached_output_nsnames_(nullptr) {
    
    @autoreleasepool {
        NSString* modelPath = [NSString stringWithUTF8String:model_path.c_str()];
        NSURL* modelURL = [NSURL fileURLWithPath:modelPath];
        
        // Create configuration
        MLModelConfiguration* configuration = [[MLModelConfiguration alloc] init];
        
        // Set compute units
        NSString* computeUnitsStr = [NSString stringWithUTF8String:computeUnitsToString(compute_units)];
        if ([computeUnitsStr isEqualToString:@"cpu"]) {
            configuration.computeUnits = MLComputeUnitsCPUOnly;
        } else if ([computeUnitsStr isEqualToString:@"cpu_and_gpu"]) {
            configuration.computeUnits = MLComputeUnitsCPUAndGPU;
        } else if ([computeUnitsStr isEqualToString:@"cpu_and_neural_engine"]) {
            configuration.computeUnits = MLComputeUnitsCPUAndNeuralEngine;
        } else {
            configuration.computeUnits = MLComputeUnitsAll;
        }
        
        NSError* error = nil;
        MLModel* model = [MLModel modelWithContentsOfURL:modelURL configuration:configuration error:&error];
        
        if (error || !model) {
            std::string errorMsg = "Failed to load CoreML model: ";
            if (error) {
                errorMsg += [[error localizedDescription] UTF8String];
            } else {
                errorMsg += "Unknown error";
            }
            throw std::runtime_error(errorMsg);
        }
        
        // CRITICAL FIX: Retain the model so it doesn't get autoreleased when the pool drains
        [model retain];
        
        // Create implementation struct with exception safety
        std::unique_ptr<CoreMLModelImpl> impl_holder(new CoreMLModelImpl());
        impl_holder->model = model;  // Now this pointer will remain valid!
        impl_holder->predictionOptions = [[MLPredictionOptions alloc] init];
        impl_holder->modelPath = modelPath;
        impl_holder->computeUnits = computeUnitsStr;
        
        try {
            // Cache all model metadata once for optimal performance
            // Use temporary assignment for exception safety
            model_ptr_ = impl_holder.get();
            cacheModelMetadata();
            
            // Only release ownership if everything succeeded
            model_ptr_ = impl_holder.release();
        } catch (...) {
            // Cleanup retained model if caching fails
            [model release];
            model_ptr_ = nullptr;
            throw;  // Re-throw the exception
        }
        
        LEAFRA_INFO() << "CoreML model loaded successfully: " << model_path;
        LEAFRA_INFO() << "  - Compute units: " << computeUnitsToString(compute_units);
        LEAFRA_INFO() << "  - Input count: " << input_names_.size();
        LEAFRA_INFO() << "  - Output count: " << output_names_.size();
    }
}

// Cache model metadata once for optimal performance
void CoreMLModel::cacheModelMetadata() {
    if (!model_ptr_) return;
    
    @autoreleasepool {
        CoreMLModelImpl* impl = static_cast<CoreMLModelImpl*>(model_ptr_);
        MLModel* model = impl->model;
        
        if (!model) return;
        
        MLModelDescription* modelDescription = [model modelDescription];
        
        // Cache input metadata
        NSDictionary<NSString*, MLFeatureDescription*>* inputDescriptions = [modelDescription inputDescriptionsByName];
        NSArray<NSString*>* inputNames = [[inputDescriptions allKeys] sortedArrayUsingSelector:@selector(compare:)];
        
        // Cache NSArray of input names for fast CoreML API access
        cached_input_nsnames_ = (__bridge_retained void*)[inputNames copy];
        
        input_names_.clear();
        input_sizes_.clear();
        input_names_.reserve([inputNames count]);
        input_sizes_.reserve([inputNames count]);
        
        for (NSString* inputName in inputNames) {
            // Cache input name for public API
            input_names_.push_back(std::string([inputName UTF8String]));
            
            // Cache input size
            MLFeatureDescription* inputDescription = inputDescriptions[inputName];
            size_t input_size = 0;
            
            if (inputDescription.type == MLFeatureTypeMultiArray) {
                MLMultiArrayConstraint* constraint = inputDescription.multiArrayConstraint;
                NSInteger size = 1;
                for (NSNumber* dim in constraint.shape) {
                    size *= [dim integerValue];
                }
                input_size = size;
            }
            
            input_sizes_.push_back(input_size);
        }
        
        // Cache output metadata
        NSDictionary<NSString*, MLFeatureDescription*>* outputDescriptions = [modelDescription outputDescriptionsByName];
        NSArray<NSString*>* outputNames = [[outputDescriptions allKeys] sortedArrayUsingSelector:@selector(compare:)];
        
        // Cache NSArray of output names for fast CoreML API access
        cached_output_nsnames_ = (__bridge_retained void*)[outputNames copy];
        
        output_names_.clear();
        output_sizes_.clear();
        output_names_.reserve([outputNames count]);
        output_sizes_.reserve([outputNames count]);
        
        for (NSString* outputName in outputNames) {
            // Cache output name for public API
            output_names_.push_back(std::string([outputName UTF8String]));
            
            // Cache output size
            MLFeatureDescription* outputDescription = outputDescriptions[outputName];
            size_t output_size = 0;
            
            if (outputDescription.type == MLFeatureTypeMultiArray) {
                MLMultiArrayConstraint* constraint = outputDescription.multiArrayConstraint;
                NSInteger size = 1;
                for (NSNumber* dim in constraint.shape) {
                    size *= [dim integerValue];
                }
                output_size = size;
            }
            
            output_sizes_.push_back(output_size);
        }
        
        // Cache model description
        model_description_ = std::string([[NSString stringWithFormat:@"Model: %@\nInputs: %lu\nOutputs: %lu", 
                                          @"CoreML Model",
                                          (unsigned long)input_names_.size(),
                                          (unsigned long)output_names_.size()] UTF8String]);
    }
}

// Destructor
CoreMLModel::~CoreMLModel() {
    if (model_ptr_) {
        @autoreleasepool {
            CoreMLModelImpl* impl = static_cast<CoreMLModelImpl*>(model_ptr_);
            
            // Release the retained MLModel object
            [impl->model release];
            
            // ARC will automatically release the Objective-C objects
            impl->model = nil;
            impl->predictionOptions = nil;
            impl->modelPath = nil;
            impl->computeUnits = nil;
            
            delete impl;
            model_ptr_ = nullptr;
        }
    }
    
    // Release cached NSArray objects (from __bridge_retained)
    if (cached_input_nsnames_) {
        CFRelease(cached_input_nsnames_);
        cached_input_nsnames_ = nullptr;
    }
    if (cached_output_nsnames_) {
        CFRelease(cached_output_nsnames_);
        cached_output_nsnames_ = nullptr;
    }
}

// Move constructor
CoreMLModel::CoreMLModel(CoreMLModel&& other) noexcept
    : model_ptr_(other.model_ptr_), cached_input_nsnames_(other.cached_input_nsnames_), cached_output_nsnames_(other.cached_output_nsnames_),
      input_names_(std::move(other.input_names_)), input_sizes_(std::move(other.input_sizes_)),
      output_names_(std::move(other.output_names_)), output_sizes_(std::move(other.output_sizes_)),
      model_description_(std::move(other.model_description_)) {
    other.model_ptr_ = nullptr;
    other.cached_input_nsnames_ = nullptr;
    other.cached_output_nsnames_ = nullptr;
}

// Move assignment
CoreMLModel& CoreMLModel::operator=(CoreMLModel&& other) noexcept {
    if (this != &other) {
        if (model_ptr_) {
            @autoreleasepool {
                CoreMLModelImpl* impl = static_cast<CoreMLModelImpl*>(model_ptr_);
                
                // Release the retained MLModel object
                [impl->model release];
                
                impl->model = nil;
                impl->predictionOptions = nil;
                impl->modelPath = nil;
                impl->computeUnits = nil;
                delete impl;
            }
        }
        
        // Release existing cached NSArray objects (from __bridge_retained)
        if (cached_input_nsnames_) {
            CFRelease(cached_input_nsnames_);
        }
        if (cached_output_nsnames_) {
            CFRelease(cached_output_nsnames_);
        }
        
        model_ptr_ = other.model_ptr_;
        cached_input_nsnames_ = other.cached_input_nsnames_;
        cached_output_nsnames_ = other.cached_output_nsnames_;
        input_names_ = std::move(other.input_names_);
        input_sizes_ = std::move(other.input_sizes_);
        output_names_ = std::move(other.output_names_);
        output_sizes_ = std::move(other.output_sizes_);
        model_description_ = std::move(other.model_description_);
        other.model_ptr_ = nullptr;
        other.cached_input_nsnames_ = nullptr;
        other.cached_output_nsnames_ = nullptr;
    }
    return *this;
}

// Check validity
bool CoreMLModel::isValid() const {
    return model_ptr_ && static_cast<CoreMLModelImpl*>(model_ptr_)->model != nil;
}

// Get description (using cached data)
std::string CoreMLModel::getDescription() const {
    return model_description_;
}

// All introspection methods now use cached data (implemented as inline functions in header)

// Optimized prediction using cached metadata
std::vector<std::vector<float> > CoreMLModel::predict(const std::vector<std::vector<float> >& inputs) {
    if (!model_ptr_) {
        throw std::runtime_error("Invalid CoreML model");
    }
    
    size_t num_inputs = inputs.size();
    size_t num_outputs = output_names_.size();
    
    if (num_inputs != input_names_.size()) {
        throw std::runtime_error("Input count mismatch: expected " + std::to_string(input_names_.size()) + ", got " + std::to_string(num_inputs));
    }
    
    // Prepare output arrays using cached sizes
    std::vector<std::vector<float> > outputs(num_outputs);
    
    for (size_t i = 0; i < num_outputs; ++i) {
        outputs[i].resize(output_sizes_[i]);
    }
    
    // Perform prediction
    if (!predict(inputs, outputs)) {
        throw std::runtime_error("CoreML prediction failed");
    }
    
    return outputs;
}

// Pre-allocated multiple inputs/outputs prediction
bool CoreMLModel::predict(const std::vector<std::vector<float> >& inputs, 
                         std::vector<std::vector<float> >& outputs) {
    if (!model_ptr_) return false;
    
    @autoreleasepool {
        CoreMLModelImpl* impl = static_cast<CoreMLModelImpl*>(model_ptr_);
        MLModel* model = impl->model;
        
        if (!model) return false;
        
        NSError* error = nil;
        
        size_t num_inputs = inputs.size();
        size_t num_outputs = outputs.size();
        
        // Verify number of inputs using cached data
        if (num_inputs != input_names_.size()) {
            LEAFRA_ERROR() << "Input count mismatch: model expects " << input_names_.size() << ", got " << num_inputs;
            return false;
        }
        
        // Create input features using cached NSString objects (no conversions!)
        NSMutableDictionary* inputFeatures = [[NSMutableDictionary alloc] init];
        NSArray<NSString*>* cachedInputNames = (__bridge NSArray<NSString*>*)cached_input_nsnames_;
        
        for (size_t i = 0; i < num_inputs; i++) {
            NSString* inputName = cachedInputNames[i];
            
            // Verify input size matches cached expected size
            if (inputs[i].size() != input_sizes_[i]) {
                LEAFRA_ERROR() << "Input[" << i << "] size mismatch: expected " << input_sizes_[i] << ", got " << inputs[i].size();
                return false;
            }
            
            // Create MLMultiArray directly using cached size (no model queries needed!)
            NSArray<NSNumber*>* shape = @[@1, @(input_sizes_[i])];  // 2D tensor: [batch_size=1, sequence_length]
            MLMultiArray* inputArray = [[MLMultiArray alloc] initWithShape:shape
                                                                  dataType:MLMultiArrayDataTypeFloat32
                                                                     error:&error];
            if (error) {
                LEAFRA_ERROR() << "Failed to create input array[" << i << "]: " << [[error localizedDescription] UTF8String];
                return false;
            }
            
            // Copy input data
            float* arrayData = (float*)[inputArray dataPointer];
            memcpy(arrayData, inputs[i].data(), inputs[i].size() * sizeof(float));
            
            inputFeatures[inputName] = [MLFeatureValue featureValueWithMultiArray:inputArray];
        }
        
        // Create feature provider
        MLDictionaryFeatureProvider* featureProvider = [[MLDictionaryFeatureProvider alloc] 
            initWithDictionary:inputFeatures error:&error];
        if (error) {
            LEAFRA_ERROR() << "Failed to create feature provider: " << [[error localizedDescription] UTF8String];
            return false;
        }
        
        // Perform prediction
        id<MLFeatureProvider> prediction = [model predictionFromFeatures:featureProvider
                                                                 options:impl->predictionOptions
                                                                   error:&error];
        if (error) {
            LEAFRA_ERROR() << "Prediction failed: " << [[error localizedDescription] UTF8String];
            return false;
        }
        
        // Verify number of outputs using cached data
        if (num_outputs != output_names_.size()) {
            LEAFRA_ERROR() << "Output count mismatch: model produces " << output_names_.size() << ", expected " << num_outputs;
            return false;
        }
        
        // Extract output data using cached NSString objects (no conversions!)
        NSArray<NSString*>* cachedOutputNames = (__bridge NSArray<NSString*>*)cached_output_nsnames_;
        
        for (size_t i = 0; i < num_outputs; i++) {
            NSString* outputName = cachedOutputNames[i];
            MLFeatureValue* outputFeature = [prediction featureValueForName:outputName];
            
            if (!outputFeature) {
                LEAFRA_ERROR() << "Failed to get output feature[" << i << "]: " << [outputName UTF8String];
                return false;
            }
            
            // Extract output data
            if (outputFeature.type == MLFeatureTypeMultiArray) {
                MLMultiArray* outputArray = outputFeature.multiArrayValue;
                
                // Check actual output tensor shape for batch dimension handling
                NSArray<NSNumber*>* outputShape = outputArray.shape;
                
                size_t expected_elements = output_sizes_[i];
                size_t actual_elements = outputArray.count;
                
                // Verify our pre-allocated output buffer size
                if (outputs[i].size() != expected_elements) {
                    LEAFRA_ERROR() << "Output[" << i << "] buffer size mismatch: expected " << expected_elements << ", got " << outputs[i].size();
                    return false;
                }
                
                // Verify actual tensor element count matches our expectation
                if (actual_elements != expected_elements) {
                    LEAFRA_ERROR() << "Output[" << i << "] tensor element count mismatch: expected " << expected_elements << ", got " << actual_elements;
                    return false;
                }
                
                // Direct memory copy works for both [1, N] and [N] tensor shapes
                // since memory layout is identical (contiguous elements)
                
                // Copy output data
                float* arrayData = (float*)[outputArray dataPointer];
                memcpy(outputs[i].data(), arrayData, outputs[i].size() * sizeof(float));
                
            } else {
                LEAFRA_ERROR() << "Unsupported output type for output[" << i << "]: " << (int)outputFeature.type;
                return false;
            }
        }
        
        LEAFRA_INFO() << "CoreML prediction completed successfully";
        LEAFRA_INFO() << "  - Number of inputs: " << num_inputs;
        LEAFRA_INFO() << "  - Number of outputs: " << num_outputs;
        
        return true;
    }
}

} // namespace leafra 