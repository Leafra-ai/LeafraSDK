#pragma once

#include "types.h"

namespace leafra {

#ifdef LEAFRA_HAS_COREML

/**
 * @brief CoreML Model Interface
 * 
 * This interface provides C++ access to CoreML models on Apple platforms.
 * The actual implementation is in Objective-C++ (leafra_coreml.mm).
 */

// C-style interface for CoreML operations (to avoid exposing Objective-C++ types)

/**
 * @brief Create and load a CoreML model
 * @param model_path Path to the .mlmodel or .mlpackage file
 * @param compute_units Compute units preference: "all", "cpuOnly", "cpuAndGPU", "cpuAndNeuralEngine"
 * @return Opaque pointer to CoreML model instance, nullptr on failure
 */
LEAFRA_API void* coreml_create_model(const char* model_path, const char* compute_units);

/**
 * @brief Get model description
 * @param model_ptr Opaque pointer to CoreML model
 * @param description Buffer to store description
 * @param max_length Maximum length of description buffer
 * @return true on success, false on failure
 */
LEAFRA_API bool coreml_get_model_description(void* model_ptr, char* description, size_t max_length);

/**
 * @brief Perform model prediction
 * @param model_ptr Opaque pointer to CoreML model
 * @param input_data Input data array
 * @param input_size Size of input data array
 * @param output_data Output data array (pre-allocated)
 * @param output_size Size of output data array
 * @return true on success, false on failure
 */
LEAFRA_API bool coreml_predict(void* model_ptr, const float* input_data, size_t input_size, 
                              float* output_data, size_t output_size);

/**
 * @brief Destroy CoreML model and free resources
 * @param model_ptr Opaque pointer to CoreML model
 */
LEAFRA_API void coreml_destroy_model(void* model_ptr);

#endif // LEAFRA_HAS_COREML

} // namespace leafra 