/*
 * ExecutTorch C++ Bridge Usage Example
 * 
 * This example demonstrates how to use ExecutTorch from C++ using the bridge.
 * To compile this example:
 * 
 * clang++ -std=c++14 -fexceptions \
 *   -I. \
 *   -framework Foundation \
 *   -F../prebuilt/executorch/apple \
 *   -framework executorch \
 *   example_usage.cpp \
 *   *.mm \
 *   -o example_usage
 */

#include "ExecutTorch_Bridge.h"
#include <iostream>
#include <vector>
#include <cstdlib>

// Helper function to check and print errors
void checkError(char* error, const std::string& operation) {
    if (error) {
        std::cerr << "Error in " << operation << ": " << error << std::endl;
        free(error);
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <path_to_model.pte>" << std::endl;
        return 1;
    }
    
    const char* modelPath = argv[1];
    
    std::cout << "ExecutTorch C++ Bridge Example" << std::endl;
    std::cout << "Model: " << modelPath << std::endl;
    std::cout << "Error Domain: " << ExecuTorchError_getDomain() << std::endl;
    
    // 1. Create and load module
    std::cout << "\n1. Creating module..." << std::endl;
    ExecuTorchModule_Handle module = ExecuTorchModule_createWithDefaultMode(modelPath);
    if (!module) {
        std::cerr << "Failed to create module" << std::endl;
        return 1;
    }
    
    std::cout << "2. Loading module..." << std::endl;
    char* error = nullptr;
    if (!ExecuTorchModule_load(module, &error)) {
        checkError(error, "loading module");
    }
    
    std::cout << "Module loaded successfully!" << std::endl;
    std::cout << "Is loaded: " << (ExecuTorchModule_isLoaded(module) ? "Yes" : "No") << std::endl;
    
    // 2. Get method names
    std::cout << "\n3. Getting method names..." << std::endl;
    size_t methodCount;
    char** methodNames = ExecuTorchModule_getMethodNames(module, &methodCount, &error);
    checkError(error, "getting method names");
    
    std::cout << "Available methods (" << methodCount << "):" << std::endl;
    for (size_t i = 0; i < methodCount; i++) {
        std::cout << "  - " << methodNames[i] << std::endl;
    }
    
    // 3. Create sample input tensor
    std::cout << "\n4. Creating sample input tensor..." << std::endl;
    float inputData[] = {1.0f, 2.0f, 3.0f, 4.0f};
    int64_t shape[] = {2, 2};
    
    ExecuTorchTensor_Handle tensor = ExecuTorchTensor_createSimple(
        inputData, shape, 2, ET_DATA_TYPE_FLOAT);
    
    if (!tensor) {
        std::cerr << "Failed to create tensor" << std::endl;
        return 1;
    }
    
    // Check tensor properties
    ExecutorchDataType_t dataType = ExecuTorchTensor_getDataType(tensor);
    size_t elementCount = ExecuTorchTensor_getElementCount(tensor);
    
    std::cout << "Tensor created:" << std::endl;
    std::cout << "  Data type: " << dataType << std::endl;
    std::cout << "  Element count: " << elementCount << std::endl;
    
    size_t shapeDims;
    const int64_t* tensorShape = ExecuTorchTensor_getShape(tensor, &shapeDims);
    std::cout << "  Shape: [";
    for (size_t i = 0; i < shapeDims; i++) {
        std::cout << tensorShape[i];
        if (i < shapeDims - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    
    // 4. Create value from tensor
    std::cout << "\n5. Creating value from tensor..." << std::endl;
    ExecuTorchValue_Handle value = ExecuTorchValue_createWithTensor(tensor);
    if (!value) {
        std::cerr << "Failed to create value" << std::endl;
        return 1;
    }
    
    std::cout << "Value created:" << std::endl;
    std::cout << "  Is tensor: " << (ExecuTorchValue_isTensor(value) ? "Yes" : "No") << std::endl;
    std::cout << "  Is scalar: " << (ExecuTorchValue_isScalar(value) ? "Yes" : "No") << std::endl;
    
    // 5. Execute forward method (if available)
    std::cout << "\n6. Attempting to execute 'forward' method..." << std::endl;
    
    // Check if forward method exists
    bool hasForward = false;
    for (size_t i = 0; i < methodCount; i++) {
        if (std::string(methodNames[i]) == "forward") {
            hasForward = true;
            break;
        }
    }
    
    if (hasForward) {
        // Load the forward method
        if (!ExecuTorchModule_loadMethod(module, "forward", &error)) {
            checkError(error, "loading forward method");
        }
        
        std::cout << "Forward method loaded: " << 
            (ExecuTorchModule_isMethodLoaded(module, "forward") ? "Yes" : "No") << std::endl;
        
        // Execute the method
        size_t outputCount;
        ExecuTorchValue_Handle* outputs = ExecuTorchModule_executeMethod(
            module, "forward", &value, 1, &outputCount, &error);
        
        if (outputs) {
            std::cout << "Execution successful!" << std::endl;
            std::cout << "Number of outputs: " << outputCount << std::endl;
            
            // Examine outputs
            for (size_t i = 0; i < outputCount; i++) {
                std::cout << "Output " << i << ":" << std::endl;
                std::cout << "  Is tensor: " << (ExecuTorchValue_isTensor(outputs[i]) ? "Yes" : "No") << std::endl;
                
                if (ExecuTorchValue_isTensor(outputs[i])) {
                    ExecuTorchTensor_Handle outTensor = ExecuTorchValue_getTensor(outputs[i]);
                    if (outTensor) {
                        size_t outShapeDims;
                        const int64_t* outShape = ExecuTorchTensor_getShape(outTensor, &outShapeDims);
                        std::cout << "  Shape: [";
                        for (size_t j = 0; j < outShapeDims; j++) {
                            std::cout << outShape[j];
                            if (j < outShapeDims - 1) std::cout << ", ";
                        }
                        std::cout << "]" << std::endl;
                        
                        size_t outElementCount = ExecuTorchTensor_getElementCount(outTensor);
                        std::cout << "  Element count: " << outElementCount << std::endl;
                    }
                }
            }
            
            // Free outputs
            ExecuTorchModule_freeOutputs(outputs, outputCount);
        } else {
            std::cout << "Execution failed";
            if (error) {
                std::cout << ": " << error;
                free(error);
            }
            std::cout << std::endl;
        }
    } else {
        std::cout << "No 'forward' method found. Try executing the first available method." << std::endl;
        
        if (methodCount > 0) {
            const char* firstMethod = methodNames[0];
            std::cout << "Trying method: " << firstMethod << std::endl;
            
            // Load and execute first method
            if (!ExecuTorchModule_loadMethod(module, firstMethod, &error)) {
                checkError(error, "loading first method");
            }
            
            size_t outputCount;
            ExecuTorchValue_Handle* outputs = ExecuTorchModule_executeMethod(
                module, firstMethod, &value, 1, &outputCount, &error);
            
            if (outputs) {
                std::cout << "Execution of '" << firstMethod << "' successful!" << std::endl;
                std::cout << "Number of outputs: " << outputCount << std::endl;
                ExecuTorchModule_freeOutputs(outputs, outputCount);
            } else {
                std::cout << "Execution failed";
                if (error) {
                    std::cout << ": " << error;
                    free(error);
                }
                std::cout << std::endl;
            }
        }
    }
    
    // 6. Test scalar value creation
    std::cout << "\n7. Testing scalar value creation..." << std::endl;
    
    ExecuTorchValue_Handle boolValue = ExecuTorchValue_createWithBoolean(true);
    ExecuTorchValue_Handle intValue = ExecuTorchValue_createWithInteger(42);
    ExecuTorchValue_Handle doubleValue = ExecuTorchValue_createWithDouble(3.14);
    ExecuTorchValue_Handle stringValue = ExecuTorchValue_createWithString("Hello ExecutTorch!");
    
    std::cout << "Boolean value: " << (ExecuTorchValue_getBoolean(boolValue) ? "true" : "false") << std::endl;
    std::cout << "Integer value: " << ExecuTorchValue_getInteger(intValue) << std::endl;
    std::cout << "Double value: " << ExecuTorchValue_getDouble(doubleValue) << std::endl;
    std::cout << "String value: " << ExecuTorchValue_getString(stringValue) << std::endl;
    
    // 7. Test tensor scalar creation
    std::cout << "\n8. Testing scalar tensor creation..." << std::endl;
    
    ExecuTorchTensor_Handle floatScalarTensor = ExecuTorchTensor_createFromFloat(2.5f);
    ExecuTorchTensor_Handle intScalarTensor = ExecuTorchTensor_createFromInt32(100);
    ExecuTorchTensor_Handle boolScalarTensor = ExecuTorchTensor_createFromBool(true);
    
    std::cout << "Float scalar tensor element count: " << ExecuTorchTensor_getElementCount(floatScalarTensor) << std::endl;
    std::cout << "Int scalar tensor element count: " << ExecuTorchTensor_getElementCount(intScalarTensor) << std::endl;
    std::cout << "Bool scalar tensor element count: " << ExecuTorchTensor_getElementCount(boolScalarTensor) << std::endl;
    
    // 8. Cleanup
    std::cout << "\n9. Cleaning up..." << std::endl;
    
    // Free method names
    ExecuTorchModule_freeMethodNames(methodNames, methodCount);
    
    // Destroy values
    ExecuTorchValue_destroy(boolValue);
    ExecuTorchValue_destroy(intValue);
    ExecuTorchValue_destroy(doubleValue);
    ExecuTorchValue_destroy(stringValue);
    ExecuTorchValue_destroy(value);
    
    // Destroy tensors
    ExecuTorchTensor_destroy(floatScalarTensor);
    ExecuTorchTensor_destroy(intScalarTensor);
    ExecuTorchTensor_destroy(boolScalarTensor);
    ExecuTorchTensor_destroy(tensor);
    
    // Destroy module
    ExecuTorchModule_destroy(module);
    
    std::cout << "Example completed successfully!" << std::endl;
    
    return 0;
} 