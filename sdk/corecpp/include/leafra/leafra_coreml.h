#pragma once

#include "types.h"
#include <string>
#include <vector>
#include <memory>

namespace leafra {

/**
 * @brief Modern C++ CoreML Model Interface
 * 
 * This class provides a RAII-based interface to CoreML models with automatic
 * resource management, type safety, and modern C++ conveniences.
 * Model metadata is cached at construction time for optimal performance.
 * 
 * Example usage:
 * 
 * // Single input/output usage (using vectors with one element)
 * try {
 *     CoreMLModel model("model.mlpackage", CoreMLModel::ComputeUnits::All);
 *     std::vector<std::vector<float> > inputs = {{1.0f, 2.0f, 3.0f}};  // Single input
 *     auto outputs = model.predict(inputs);  // Returns vector with one output
 *     std::vector<float>& result = outputs[0];  // Get the single output
 *     // Use result...
 * } catch (const std::exception& e) {
 *     // Handle error: e.what()
 * }
 * 
 * // Multiple inputs/outputs with model introspection
 * CoreMLModel model("complex_model.mlpackage");
 * 
 * // Get model information (cached, very fast)
 * size_t num_inputs = model.getInputCount();    // e.g., 2
 * size_t num_outputs = model.getOutputCount();  // e.g., 1
 * std::string input0_name = model.getInputName(0);  // e.g., "features"
 * size_t input0_size = model.getInputSize(0);       // e.g., 512
 * 
 * // Prepare inputs
 * std::vector<std::vector<float> > inputs = {
 *     std::vector<float>(512, 1.0f),  // Input 0: 512 features
 *     std::vector<float>(128, 0.5f)   // Input 1: 128 features
 * };
 * 
 * // Predict (optimized with cached metadata)
 * auto outputs = model.predict(inputs);  // Returns std::vector<std::vector<float> >
 * 
 * // For performance-critical code with pre-allocated outputs:
 * std::vector<std::vector<float> > output_buffers = {
 *     std::vector<float>(10)  // Pre-allocate output buffer
 * };
 * bool success = model.predict(inputs, output_buffers);  // No allocation
 */
class LEAFRA_API CoreMLModel {
public:
    enum class ComputeUnits {
        CPUOnly,              // CPU only
        All,                  // All available (CPU + GPU + Neural Engine)
        CPUAndGPU,           // CPU and GPU
        CPUAndNeuralEngine   // CPU and Neural Engine
    };

    /**
     * @brief Construct CoreML model from file
     * @param model_path Path to .mlpackage or .mlmodel file
     * @param compute_units Compute units to use for inference
     * @throws std::runtime_error if model loading fails
     */
    CoreMLModel(const std::string& model_path, ComputeUnits compute_units = ComputeUnits::All);

    /**
     * @brief Destructor - automatically cleans up resources
     */
    ~CoreMLModel();

    // Non-copyable but movable
    CoreMLModel(const CoreMLModel&) = delete;
    CoreMLModel& operator=(const CoreMLModel&) = delete;
    CoreMLModel(CoreMLModel&&) noexcept;
    CoreMLModel& operator=(CoreMLModel&&) noexcept;

    /**
     * @brief Check if model is valid and ready for inference
     */
    bool isValid() const;

    /**
     * @brief Get model description
     */
    std::string getDescription() const;

    // Model introspection (cached for optimal performance)
    size_t getInputCount() const { return input_names_.size(); }
    size_t getOutputCount() const { return output_names_.size(); }
    std::string getInputName(size_t index) const { return index < input_names_.size() ? input_names_[index] : ""; }
    std::string getOutputName(size_t index) const { return index < output_names_.size() ? output_names_[index] : ""; }
    size_t getInputSize(size_t index) const { return index < input_sizes_.size() ? input_sizes_[index] : 0; }
    size_t getOutputSize(size_t index) const { return index < output_sizes_.size() ? output_sizes_[index] : 0; }
    const std::vector<std::string>& getInputNames() const { return input_names_; }
    const std::vector<std::string>& getOutputNames() const { return output_names_; }
    const std::vector<size_t>& getInputSizes() const { return input_sizes_; }
    const std::vector<size_t>& getOutputSizes() const { return output_sizes_; }

    // Prediction methods (handles both single and multiple inputs/outputs)
    std::vector<std::vector<float> > predict(const std::vector<std::vector<float> >& inputs);

    // Advanced prediction with pre-allocated outputs
    bool predict(const std::vector<std::vector<float> >& inputs, 
                std::vector<std::vector<float> >& outputs);

private:
    void* model_ptr_;  // Opaque pointer to implementation
    
    // Cached model metadata for optimal performance
    std::vector<std::string> input_names_;   // For public API
    std::vector<std::string> output_names_;  // For public API
    std::vector<size_t> input_sizes_;
    std::vector<size_t> output_sizes_;
    std::string model_description_;
    
    // Cached NSString objects for internal CoreML API calls (avoid conversions)
    void* cached_input_nsnames_;   // NSArray<NSString*>*
    void* cached_output_nsnames_;  // NSArray<NSString*>*
    
    // Helper methods
    static const char* computeUnitsToString(ComputeUnits units);
    void cacheModelMetadata();  // Called once in constructor
};

} // namespace leafra 