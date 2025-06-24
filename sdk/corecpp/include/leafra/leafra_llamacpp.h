#ifndef LEAFRA_LLAMACPP_H
#define LEAFRA_LLAMACPP_H

#ifdef LEAFRA_HAS_LLAMACPP

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <cstdint>

// Forward declarations
struct llama_model;
struct llama_context;
struct llama_sampling_context;
struct LLMConfig;

namespace leafra {
namespace llamacpp {

/**
 * @brief Configuration for LlamaCpp model loading and inference
 */
struct LlamaCppConfig {
    // Model file path
    std::string model_path;
    
    // Context and generation parameters
    int32_t n_ctx = 4096;           // Context size (max tokens)
    int32_t n_batch = 512;          // Batch size for prompt processing
    int32_t n_ubatch = 512;         // Physical batch size for prompt processing
    int32_t n_threads = -1;         // Number of threads (-1 = auto)
    int32_t n_threads_batch = -1;   // Number of threads for batch processing (-1 = auto)
    
    // Generation parameters
    int32_t n_predict = 128;        // Number of tokens to predict
    float temperature = 0.8f;       // Sampling temperature
    float top_p = 0.9f;            // Nucleus sampling probability
    int32_t top_k = 40;            // Top-k sampling
    float min_p = 0.05f;           // Minimum probability for sampling
    float repeat_penalty = 1.1f;    // Repetition penalty
    int32_t repeat_last_n = 64;     // Last n tokens to consider for repetition penalty
    
    // Model loading parameters
    bool use_mmap = true;           // Use memory mapping for model loading
    bool use_mlock = false;         // Use memory locking
    bool numa = false;              // NUMA optimization
    int32_t n_gpu_layers = -1;       // Number of layers to offload to GPU (-1 = auto --> fully offload to gpu )
    
    // Advanced options
    bool verbose_prompt = false;    // Print prompt before generation
    bool debug_mode = false;        // Enable debug output
    
    // Sampling parameters
    int32_t seed = -1;              // Random seed (-1 = random)
    float tfs_z = 1.0f;            // Tail free sampling - not enabled by default
    float typical_p = 1.0f;        // Typical sampling - not enabled by default
    
    // Default constructor
    LlamaCppConfig() = default;
    
    // Constructor with model path
    explicit LlamaCppConfig(const std::string& model_path_param) 
        : model_path(model_path_param) {}
};

/**
 * @brief Chat message structure for conversation formatting
 */
struct ChatMessage {
    std::string role;       // "system", "user", "assistant", etc.
    std::string content;    // Message content
    
    ChatMessage() = default;
    ChatMessage(const std::string& r, const std::string& c) : role(r), content(c) {}
};

/**
 * @brief Token generation callback function type
 * @param token The generated token text
 * @param is_final Whether this is the final token
 * @return true to continue generation, false to stop
 */
using TokenCallback = std::function<bool(const std::string& token, bool is_final)>;

/**
 * @brief Statistics about model and generation
 */
struct GenerationStats {
    int32_t prompt_tokens = 0;      // Number of tokens in prompt
    int32_t generated_tokens = 0;   // Number of tokens generated
    double prompt_eval_time = 0.0;  // Time to evaluate prompt (ms)
    double generation_time = 0.0;   // Time to generate tokens (ms)
    double tokens_per_second = 0.0; // Generation speed (tokens/sec)
    
    GenerationStats() = default;
};

/**
 * @brief LlamaCpp model wrapper for text generation
 */
class LlamaCppModel {
public:
    LlamaCppModel();
    ~LlamaCppModel();
    
    // Non-copyable but movable
    LlamaCppModel(const LlamaCppModel&) = delete;
    LlamaCppModel& operator=(const LlamaCppModel&) = delete;
    LlamaCppModel(LlamaCppModel&&) noexcept;
    LlamaCppModel& operator=(LlamaCppModel&&) noexcept;
    
    /**
     * @brief Load model from file with specified configuration
     * @param config Model configuration
     * @return true if successful, false otherwise
     */
    bool load_model(const LlamaCppConfig& config);
    
    /**
     * @brief Check if model is loaded and ready
     * @return true if model is loaded, false otherwise
     */
    bool is_loaded() const;
    
    /**
     * @brief Unload the current model and free resources
     */
    void unload();
    
    /**
     * @brief Generate text from prompt
     * @param prompt Input prompt text
     * @param max_tokens Maximum number of tokens to generate (0 = use config default)
     * @return Generated text or empty string on error
     */
    std::string generate_text(const std::string& prompt, int32_t max_tokens = 0);
    
    /**
     * @brief Generate text with streaming callback
     * @param prompt Input prompt text
     * @param callback Function called for each generated token
     * @param max_tokens Maximum number of tokens to generate (0 = use config default)
     * @return true if generation completed successfully
     */
    bool generate_text_stream(
        const std::string& prompt,
        TokenCallback callback,
        int32_t max_tokens = 0
    );
    
    /**
     * @brief Continue text generation from current context
     * @param additional_prompt Additional text to append to context
     * @param max_tokens Maximum number of tokens to generate
     * @return Generated text or empty string on error
     */
    std::string continue_generation(const std::string& additional_prompt = "", int32_t max_tokens = 0);
    
    /**
     * @brief Reset the conversation context
     */
    void reset_context();
    
    /**
     * @brief Tokenize text into token IDs
     * @param text Input text to tokenize
     * @param add_special Add special tokens (BOS/EOS)
     * @return Vector of token IDs
     */
    std::vector<int32_t> tokenize(const std::string& text, bool add_special = false);
    
    /**
     * @brief Detokenize token IDs back to text
     * @param tokens Vector of token IDs
     * @return Detokenized text
     */
    std::string detokenize(const std::vector<int32_t>& tokens);
    
    /**
     * @brief Get token text for a specific token ID
     * @param token_id Token ID
     * @return Token text or empty string if invalid
     */
    std::string get_token_text(int32_t token_id);
    
    /**
     * @brief Calculate perplexity for given text
     * @param text Input text
     * @return Perplexity value or -1.0 on error
     */
    double calculate_perplexity(const std::string& text);
    
    /**
     * @brief Get embeddings for text (if supported by model)
     * @param text Input text
     * @return Vector of embeddings or empty vector if not supported
     */
    std::vector<float> get_embeddings(const std::string& text);
    
    /**
     * @brief Get vocabulary size
     * @return Number of tokens in vocabulary
     */
    int32_t get_vocab_size() const;
    
    /**
     * @brief Get context size (maximum tokens)
     * @return Context size in tokens
     */
    int32_t get_context_size() const;
    
    /**
     * @brief Get current context usage
     * @return Number of tokens currently in context
     */
    int32_t get_context_used() const;
    
    /**
     * @brief Get model configuration
     * @return Current model configuration
     */
    const LlamaCppConfig& get_config() const;
    
    /**
     * @brief Update generation parameters without reloading model
     * @param config New configuration (only generation parameters will be updated)
     */
    void update_generation_config(const LlamaCppConfig& config);
    
    /**
     * @brief Get statistics from last generation
     * @return Generation statistics
     */
    GenerationStats get_last_stats() const;
    
    /**
     * @brief Get model information
     * @return String with model details
     */
    std::string get_model_info() const;
    
    /**
     * @brief Get last error message
     * @return Error message or empty string if no error
     */
    std::string get_last_error() const;
    
    /**
     * @brief Check if model supports embeddings
     * @return true if embeddings are supported
     */
    bool supports_embeddings() const;
    
    /**
     * @brief Set system prompt (if model supports it)
     * @param system_prompt System prompt text
     * @return true if successful
     */
    bool set_system_prompt(const std::string& system_prompt);

    /**
     * @brief Generate response for a chat conversation
     * @param messages Vector of chat messages (conversation history)
     * @param max_tokens Maximum number of tokens to generate
     * @return Generated response or empty string on error
     */
    std::string generate_chat_response(const std::vector<ChatMessage>& messages, int32_t max_tokens = 0);
    
    /**
     * @brief Generate response for a chat conversation with streaming
     * @param messages Vector of chat messages (conversation history)
     * @param callback Function called for each generated token
     * @param max_tokens Maximum number of tokens to generate
     * @return true if generation completed successfully
     */
    bool generate_chat_response_stream(
        const std::vector<ChatMessage>& messages,
        TokenCallback callback,
        int32_t max_tokens = 0
    );
    
    /**
     * @brief Format chat messages using the model's built-in chat template
     * @param messages Vector of chat messages
     * @param add_generation_prompt Whether to add assistant prefix for generation
     * @return Formatted prompt string
     */
    std::string format_chat_prompt(const std::vector<ChatMessage>& messages, bool add_generation_prompt = true);
    
    /**
     * @brief Set custom chat template for this model
     * @param template_name Name of the template (e.g., "chatml", "llama2", "llama3")
     * @return true if successful
     */
    bool set_chat_template(const std::string& template_name);
    
    /**
     * @brief Get the model's default chat template name
     * @return Chat template name or empty string if not available
     */
    std::string get_chat_template() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

/**
 * @brief Global LlamaCpp initialization and cleanup
 */
namespace global {
    /**
     * @brief Initialize LlamaCpp library
     * Must be called before using any LlamaCpp functionality
     * @param log_disable Disable internal llama.cpp logging
     * @return true if successful, false otherwise
     */
    bool initialize(bool log_disable = false);
    
    /**
     * @brief Cleanup LlamaCpp library
     * Should be called when done using LlamaCpp functionality
     */
    void cleanup();
    
    /**
     * @brief Check if LlamaCpp is initialized
     * @return true if initialized, false otherwise
     */
    bool is_initialized();
    
    /**
     * @brief Get LlamaCpp version information
     * @return Version string
     */
    std::string get_version();
    
    /**
     * @brief Get system information relevant to LlamaCpp
     * @return System info string
     */
    std::string get_system_info();
}

/**
 * @brief Utility functions for LlamaCpp integration
 */
namespace utils {
    /**
     * @brief Check if a model file exists and is valid
     * @param model_path Path to model file
     * @return true if file exists and appears to be a valid GGUF model
     */
    bool is_valid_model_file(const std::string& model_path);
    
    /**
     * @brief Get recommended configuration for a model
     * @param model_path Path to model file
     * @return Recommended configuration or default config if analysis fails
     */
    LlamaCppConfig get_recommended_config(const std::string& model_path);
    
    /**
     * @brief Convert LLMConfig to LlamaCppConfig
     * @param llm_config General LLM configuration from SDK
     * @return LlamaCppConfig with equivalent parameters
     */
    LlamaCppConfig from_llm_config(const struct LLMConfig& llm_config);
    
    /**
     * @brief Get list of available built-in chat templates
     * @return Vector of template names that can be used with set_chat_template()
     */
    std::vector<std::string> get_available_chat_templates();
}

} // namespace llamacpp
} // namespace leafra

#endif // LEAFRA_HAS_LLAMACPP

#endif // LEAFRA_LLAMACPP_H 