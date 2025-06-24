#include "leafra/leafra_llamacpp.h"
#include "leafra/types.h"

#ifdef LEAFRA_HAS_LLAMACPP

#include "leafra/logger.h"

// Include LlamaCpp headers
#include <llama.h>
#include <ggml.h>

#include <iostream>
#include <thread>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <random>
#include <cstring>
#include <ctime>

namespace leafra {
namespace llamacpp {

// Global state
static bool g_initialized = false;
static std::mt19937 g_rng;



// LlamaCppModel::Impl definition
class LlamaCppModel::Impl {
public:
    Impl() : context_(nullptr), model_(nullptr), vocab_(nullptr), sampler_(nullptr), context_used_(0) {}
    ~Impl() {
        cleanup();
    }
    
    bool load_model(const LlamaCppConfig& config) {
        config_ = config;
        
        // Initialize llama backend if not already done
        if (!global::is_initialized()) {
            last_error_ = "LlamaCpp library not initialized. Call leafra::llamacpp::global::initialize() first.";
            LEAFRA_ERROR() << last_error_;
            return false;
        }
        
        // Validate model file
        if (!utils::is_valid_model_file(config.model_path)) {
            last_error_ = "Invalid or missing model file: " + config.model_path;
            LEAFRA_ERROR() << last_error_;
            return false;
        }
        
        // Clean up any existing model
        cleanup();
        
        // Set up model parameters
        llama_model_params model_params = llama_model_default_params();
        model_params.n_gpu_layers = config.n_gpu_layers;
        model_params.use_mmap = config.use_mmap;
        model_params.use_mlock = config.use_mlock;
        model_params.vocab_only = false;
        
        // Load model
        model_ = llama_model_load_from_file(config.model_path.c_str(), model_params);
        if (!model_) {
            last_error_ = "Failed to load model from: " + config.model_path;
            LEAFRA_ERROR() << last_error_;
            return false;
        }
        
        // Set up context parameters
        llama_context_params ctx_params = llama_context_default_params();
        ctx_params.n_ctx = config.n_ctx;
        ctx_params.n_batch = config.n_batch;
        ctx_params.n_ubatch = config.n_ubatch;
        ctx_params.n_threads = config.n_threads > 0 ? config.n_threads : static_cast<int32_t>(std::thread::hardware_concurrency());
        ctx_params.n_threads_batch = config.n_threads_batch > 0 ? config.n_threads_batch : ctx_params.n_threads;
        ctx_params.embeddings = true; // Enable embeddings support
        
        // Create context
        context_ = llama_init_from_model(model_, ctx_params);
        if (!context_) {
            last_error_ = "Failed to create context";
            LEAFRA_ERROR() << last_error_;
            llama_model_free(model_);
            model_ = nullptr;
            return false;
        }
        
        // Get vocab info
        vocab_ = llama_model_get_vocab(model_);
        
        // Cache model info
        vocab_size_ = llama_vocab_n_tokens(vocab_);
        context_size_ = llama_n_ctx(context_);
        context_used_ = 0;
        
        // Initialize sampling chain
        auto sparams = llama_sampler_chain_default_params();
        sampler_ = llama_sampler_chain_init(sparams);
        
        // Add sampling components based on configuration
        if (config.temperature <= 0.0f) {
            // Greedy sampling
            llama_sampler_chain_add(sampler_, llama_sampler_init_greedy());
        } else {
            // Temperature-based sampling chain
            if (config.top_k > 0) {
                llama_sampler_chain_add(sampler_, llama_sampler_init_top_k(config.top_k));
            }
            if (config.top_p < 1.0f) {
                llama_sampler_chain_add(sampler_, llama_sampler_init_top_p(config.top_p, 1));
            }
            if (config.min_p > 0.0f) {
                llama_sampler_chain_add(sampler_, llama_sampler_init_min_p(config.min_p, 1));
            }
            if (config.tfs_z < 1.0f) {
                llama_sampler_chain_add(sampler_, llama_sampler_init_top_n_sigma(config.tfs_z));
            }
            if (config.typical_p < 1.0f) {
                llama_sampler_chain_add(sampler_, llama_sampler_init_typical(config.typical_p, 1));
            }
            
            // Add temperature
            llama_sampler_chain_add(sampler_, llama_sampler_init_temp(config.temperature));
            
            // Add repetition penalty
            if (config.repeat_penalty != 1.0f) {
                llama_sampler_chain_add(sampler_, llama_sampler_init_penalties(
                    config.repeat_last_n,  // penalty_last_n
                    config.repeat_penalty, // penalty_repeat
                    0.0f,                  // penalty_freq
                    0.0f                   // penalty_present
                ));
            }
            
            // Final sampling step
            uint32_t seed = config.seed >= 0 ? static_cast<uint32_t>(config.seed) : static_cast<uint32_t>(std::time(nullptr));
            llama_sampler_chain_add(sampler_, llama_sampler_init_dist(seed));
        }
        
        // Set up RNG
        this->seed_rng(config.seed);
        
        LEAFRA_INFO() << "✅ LlamaCpp model loaded successfully";
        LEAFRA_INFO() << "  - Model: " << config.model_path;
        LEAFRA_INFO() << "  - Vocabulary size: " << vocab_size_;
        LEAFRA_INFO() << "  - Context size: " << context_size_;
        LEAFRA_INFO() << "  - Threads: " << ctx_params.n_threads;
        LEAFRA_INFO() << "  - GPU layers: " << config.n_gpu_layers;
        
        return true;
    }
    
    void cleanup() {
        if (sampler_) {
            llama_sampler_free(sampler_);
            sampler_ = nullptr;
        }
        if (context_) {
            llama_free(context_);
            context_ = nullptr;
        }
        if (model_) {
            llama_model_free(model_);
            model_ = nullptr;
        }
        vocab_ = nullptr;
        context_used_ = 0;
        last_error_.clear();
        last_stats_ = GenerationStats{};
    }
    
    bool is_loaded() const {
        return model_ != nullptr && context_ != nullptr && vocab_ != nullptr;
    }
    
    void unload() {
        cleanup();
    }
    
    std::string generate_text(const std::string& prompt, int32_t max_tokens) {
        if (!is_loaded()) {
            last_error_ = "Model not loaded";
            return "";
        }
        
        std::string result;
        bool success = generate_text_stream(prompt, 
            [&result](const std::string& token, bool is_final) {
                if (!is_final) result += token;
                return true; // Continue generation
            }, max_tokens);
            
        return success ? result : "";
    }
    
    bool generate_text_stream(const std::string& prompt, TokenCallback callback, int32_t max_tokens) {
        if (!is_loaded()) {
            last_error_ = "Model not loaded";
            return false;
        }
        
        // Use config default if max_tokens is 0
        if (max_tokens <= 0) {
            max_tokens = config_.n_predict;
        }
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Tokenize prompt
        auto tokens = tokenize(prompt, true);
        if (tokens.empty()) {
            last_error_ = "Failed to tokenize prompt";
            return false;
        }
        
        // Make sure we don't exceed context size
        if (static_cast<int32_t>(tokens.size()) + max_tokens > context_size_) {
            max_tokens = context_size_ - static_cast<int32_t>(tokens.size());
            if (max_tokens <= 0) {
                last_error_ = "Prompt too long for context size";
                return false;
            }
        }
        
        // Clear context and evaluate prompt
        llama_memory_clear(llama_get_memory(context_), true);
        context_used_ = 0;
        
        // Create batch for processing
        llama_batch batch = llama_batch_init(config_.n_batch, 0, 1);
        
        auto prompt_start = std::chrono::high_resolution_clock::now();
        
        // Process prompt tokens in batches
        for (size_t i = 0; i < tokens.size(); i += config_.n_batch) {
            this->batch_clear(batch);
            
            size_t batch_size = std::min(static_cast<size_t>(config_.n_batch), tokens.size() - i);
            for (size_t j = 0; j < batch_size; ++j) {
                this->batch_add(batch, tokens[i + j], static_cast<llama_pos>(i + j), {0}, j == batch_size - 1);
            }
            
            if (llama_decode(context_, batch) != 0) {
                last_error_ = "Failed to evaluate prompt batch";
                llama_batch_free(batch);
                return false;
            }
            
            context_used_ += static_cast<int32_t>(batch_size);
        }
        
        auto prompt_end = std::chrono::high_resolution_clock::now();
        double prompt_eval_time = std::chrono::duration<double, std::milli>(prompt_end - prompt_start).count();
        
        // Generate tokens
        std::vector<int32_t> generated_tokens;
        generated_tokens.reserve(max_tokens);
        
        auto generation_start = std::chrono::high_resolution_clock::now();
        
        for (int32_t i = 0; i < max_tokens; ++i) {
            // Sample next token using modern sampling API
            llama_token next_token = llama_sampler_sample(sampler_, context_, -1);
            
            // Accept the token (updates internal state of samplers)
            llama_sampler_accept(sampler_, next_token);
            
            // Check for EOS token
            if (llama_vocab_is_eog(vocab_, next_token)) {
                if (callback) {
                    callback("", true); // Signal end of generation
                }
                break;
            }
            
            generated_tokens.push_back(next_token);
            
            // Get token text and call callback
            std::string token_text = get_token_text(next_token);
            if (callback && !callback(token_text, false)) {
                if (callback) {
                    callback("", true); // Signal end of generation (user stopped)
                }
                break; // User requested stop
            }
            
            // Add token to context for next iteration
            this->batch_clear(batch);
            this->batch_add(batch, next_token, context_used_, {0}, true);
            
            if (llama_decode(context_, batch) != 0) {
                last_error_ = "Failed to evaluate generated token";
                break;
            }
            
            context_used_++;
        }
        
        // Call final callback if we completed the loop without EOS or user stop
        if (callback) {
            callback("", true); // Signal end of generation (max tokens reached)
        }
        
        llama_batch_free(batch);
        
        auto generation_end = std::chrono::high_resolution_clock::now();
        double generation_time = std::chrono::duration<double, std::milli>(generation_end - generation_start).count();
        
        // Update statistics
        last_stats_.prompt_tokens = static_cast<int32_t>(tokens.size());
        last_stats_.generated_tokens = static_cast<int32_t>(generated_tokens.size());
        last_stats_.prompt_eval_time = prompt_eval_time;
        last_stats_.generation_time = generation_time;
        last_stats_.tokens_per_second = generation_time > 0 ? (generated_tokens.size() * 1000.0) / generation_time : 0.0;
        
        if (config_.debug_mode) {
            LEAFRA_DEBUG() << "Generation stats:";
            LEAFRA_DEBUG() << "  - Prompt tokens: " << last_stats_.prompt_tokens;
            LEAFRA_DEBUG() << "  - Generated tokens: " << last_stats_.generated_tokens;
            LEAFRA_DEBUG() << "  - Prompt eval time: " << last_stats_.prompt_eval_time << "ms";
            LEAFRA_DEBUG() << "  - Generation time: " << last_stats_.generation_time << "ms";
            LEAFRA_DEBUG() << "  - Speed: " << last_stats_.tokens_per_second << " tokens/sec";
        }
        
        return true;
    }
    
    std::string continue_generation(const std::string& additional_prompt, int32_t max_tokens) {
        if (!is_loaded()) {
            last_error_ = "Model not loaded";
            return "";
        }
        
        // If there's additional prompt, add it to context
        if (!additional_prompt.empty()) {
            auto tokens = tokenize(additional_prompt, false);
            
            // Check context space
            if (context_used_ + static_cast<int32_t>(tokens.size()) >= context_size_) {
                last_error_ = "Not enough context space for additional prompt";
                return "";
            }
            
            // Add tokens to context
            llama_batch batch = llama_batch_init(config_.n_batch, 0, 1);
            
            for (size_t i = 0; i < tokens.size(); i += config_.n_batch) {
                this->batch_clear(batch);
                
                size_t batch_size = std::min(static_cast<size_t>(config_.n_batch), tokens.size() - i);
                for (size_t j = 0; j < batch_size; ++j) {
                    this->batch_add(batch, tokens[i + j], context_used_ + static_cast<llama_pos>(i + j), {0}, j == batch_size - 1);
                }
                
                if (llama_decode(context_, batch) != 0) {
                    last_error_ = "Failed to evaluate additional prompt";
                    llama_batch_free(batch);
                    return "";
                }
                
                context_used_ += static_cast<int32_t>(batch_size);
            }
            
            llama_batch_free(batch);
        }
        
        // Continue generation from current context
        std::string result;
        bool success = generate_text_stream("", 
            [&result](const std::string& token, bool is_final) {
                if (!is_final) result += token;
                return true;
            }, max_tokens);
            
        return success ? result : "";
    }
    
    void reset_context() {
        if (context_) {
            llama_memory_clear(llama_get_memory(context_), true);
            context_used_ = 0;
        }
    }
    
    std::vector<int32_t> tokenize(const std::string& text, bool add_special) {
        if (!is_loaded()) {
            return {};
        }
        
        const int32_t max_tokens = static_cast<int32_t>(text.length()) + (add_special ? 2 : 0);
        std::vector<llama_token> tokens(max_tokens);
        
        int32_t n_tokens = llama_tokenize(
            vocab_,
            text.c_str(),
            static_cast<int32_t>(text.length()),
            tokens.data(),
            max_tokens,
            add_special,
            false // parse special tokens in text
        );
        
        if (n_tokens < 0) {
            // Buffer too small, try with larger buffer
            tokens.resize(-n_tokens);
            n_tokens = llama_tokenize(
                vocab_,
                text.c_str(),
                static_cast<int32_t>(text.length()),
                tokens.data(),
                static_cast<int32_t>(tokens.size()),
                add_special,
                false
            );
        }
        
        if (n_tokens < 0) {
            last_error_ = "Failed to tokenize text";
            return {};
        }
        
        tokens.resize(n_tokens);
        return std::vector<int32_t>(tokens.begin(), tokens.end());
    }
    
    std::string detokenize(const std::vector<int32_t>& tokens) {
        if (!is_loaded() || tokens.empty()) {
            return "";
        }
        
        std::string result;
        result.reserve(tokens.size() * 4); // Rough estimate
        
        // Use the same approach as get_token_text but for multiple tokens
        // This is more reliable than llama_detokenize
        for (int32_t token : tokens) {
            char buffer[256];
            int32_t len = llama_token_to_piece(vocab_, token, buffer, sizeof(buffer), 0, false);
            if (len > 0 && len < static_cast<int32_t>(sizeof(buffer))) {
                result.append(buffer, len);
            }
        }
        
        return result;
    }
    
    std::string get_token_text(int32_t token_id) {
        if (!is_loaded()) {
            return "";
        }
        
        char buffer[256];
        int32_t len = llama_token_to_piece(vocab_, token_id, buffer, sizeof(buffer), 0, false);
        if (len > 0 && len < static_cast<int32_t>(sizeof(buffer))) {
            return std::string(buffer, len);
        }
        
        return "";
    }
    
    double calculate_perplexity(const std::string& text) {
        if (!is_loaded()) {
            last_error_ = "Model not loaded";
            return -1.0;
        }
        
        auto tokens = tokenize(text, true);
        if (tokens.empty()) {
            last_error_ = "Failed to tokenize text for perplexity calculation";
            return -1.0;
        }
        
        // Save current context state
        const size_t state_size = llama_state_get_size(context_);
        std::vector<uint8_t> saved_state(state_size);
        llama_state_get_data(context_, saved_state.data(), state_size);
        
        // Clear context for perplexity calculation
        llama_memory_clear(llama_get_memory(context_), true);
        
        double total_log_prob = 0.0;
        int32_t valid_tokens = 0;
        
        llama_batch batch = llama_batch_init(1, 0, 1);
        
        for (size_t i = 0; i < tokens.size(); ++i) {
            this->batch_clear(batch);
            this->batch_add(batch, tokens[i], static_cast<llama_pos>(i), {0}, true);
            
            if (llama_decode(context_, batch) != 0) {
                last_error_ = "Failed to evaluate token for perplexity";
                llama_batch_free(batch);
                // Restore context state
                llama_state_set_data(context_, saved_state.data(), state_size);
                return -1.0;
            }
            
            if (i > 0) { // Skip the first token (no previous context)
                const float* logits = llama_get_logits_ith(context_, 0);
                if (logits) {
                    float prob = logits[tokens[i]];
                    
                    // Apply softmax to get actual probability
                    float max_logit = *std::max_element(logits, logits + vocab_size_);
                    float sum_exp = 0.0f;
                    for (int32_t j = 0; j < vocab_size_; ++j) {
                        sum_exp += std::exp(logits[j] - max_logit);
                    }
                    prob = std::exp(prob - max_logit) / sum_exp;
                    
                    if (prob > 0.0f) {
                        total_log_prob += std::log(prob);
                        valid_tokens++;
                    }
                }
            }
        }
        
        llama_batch_free(batch);
        
        // Restore context state
        llama_state_set_data(context_, saved_state.data(), state_size);
        
        if (valid_tokens == 0) {
            last_error_ = "No valid tokens for perplexity calculation";
            return -1.0;
        }
        
        double avg_log_prob = total_log_prob / valid_tokens;
        return std::exp(-avg_log_prob);
    }
    
    std::vector<float> get_embeddings(const std::string& text) {
        if (!is_loaded()) {
            last_error_ = "Model not loaded";
            return {};
        }
        
        auto tokens = tokenize(text, true);
        if (tokens.empty()) {
            last_error_ = "Failed to tokenize text for embeddings";
            return {};
        }
        
        // Save current context state
        const size_t state_size = llama_state_get_size(context_);
        std::vector<uint8_t> saved_state(state_size);
        llama_state_get_data(context_, saved_state.data(), state_size);
        
        // Clear context for embedding calculation
        llama_memory_clear(llama_get_memory(context_), true);
        
        llama_batch batch = llama_batch_init(config_.n_batch, 0, 1);
        
        // Process all tokens
        for (size_t i = 0; i < tokens.size(); i += config_.n_batch) {
            this->batch_clear(batch);
            
            size_t batch_size = std::min(static_cast<size_t>(config_.n_batch), tokens.size() - i);
            for (size_t j = 0; j < batch_size; ++j) {
                this->batch_add(batch, tokens[i + j], static_cast<llama_pos>(i + j), {0}, j == batch_size - 1);
            }
            
            if (llama_decode(context_, batch) != 0) {
                last_error_ = "Failed to evaluate tokens for embeddings";
                llama_batch_free(batch);
                // Restore context state
                llama_state_set_data(context_, saved_state.data(), state_size);
                return {};
            }
        }
        
        // Get embeddings
        const float* embeddings = llama_get_embeddings(context_);
        if (!embeddings) {
            last_error_ = "Model does not support embeddings";
            llama_batch_free(batch);
            // Restore context state
            llama_state_set_data(context_, saved_state.data(), state_size);
            return {};
        }
        
        const int32_t n_embd = llama_model_n_embd(model_);
        std::vector<float> result(embeddings, embeddings + n_embd);
        
        llama_batch_free(batch);
        
        // Restore context state
        llama_state_set_data(context_, saved_state.data(), state_size);
        
        return result;
    }
    
    int32_t get_vocab_size() const {
        return vocab_size_;
    }
    
    int32_t get_context_size() const {
        return context_size_;
    }
    
    int32_t get_context_used() const {
        return context_used_;
    }
    
    const LlamaCppConfig& get_config() const {
        return config_;
    }
    
    void update_generation_config(const LlamaCppConfig& config) {
        // Update only generation parameters
        config_.n_predict = config.n_predict;
        config_.temperature = config.temperature;
        config_.top_p = config.top_p;
        config_.top_k = config.top_k;
        config_.min_p = config.min_p;
        config_.repeat_penalty = config.repeat_penalty;
        config_.repeat_last_n = config.repeat_last_n;
        config_.verbose_prompt = config.verbose_prompt;
        config_.debug_mode = config.debug_mode;
        config_.seed = config.seed;
        config_.tfs_z = config.tfs_z;
        config_.typical_p = config.typical_p;
        
        // Re-seed RNG if seed changed
        this->seed_rng(config.seed);
        
        // Recreate sampler with new parameters
        if (sampler_) {
            llama_sampler_free(sampler_);
            sampler_ = nullptr;
        }
        
        auto sparams = llama_sampler_chain_default_params();
        sampler_ = llama_sampler_chain_init(sparams);
        
        // Add sampling components based on new configuration
        if (config_.temperature <= 0.0f) {
            llama_sampler_chain_add(sampler_, llama_sampler_init_greedy());
        } else {
            if (config_.top_k > 0) {
                llama_sampler_chain_add(sampler_, llama_sampler_init_top_k(config_.top_k));
            }
            if (config_.top_p < 1.0f) {
                llama_sampler_chain_add(sampler_, llama_sampler_init_top_p(config_.top_p, 1));
            }
            if (config_.min_p > 0.0f) {
                llama_sampler_chain_add(sampler_, llama_sampler_init_min_p(config_.min_p, 1));
            }
            if (config_.tfs_z < 1.0f) {
                llama_sampler_chain_add(sampler_, llama_sampler_init_top_n_sigma(config_.tfs_z));
            }
            if (config_.typical_p < 1.0f) {
                llama_sampler_chain_add(sampler_, llama_sampler_init_typical(config_.typical_p, 1));
            }
            
            llama_sampler_chain_add(sampler_, llama_sampler_init_temp(config_.temperature));
            
            if (config_.repeat_penalty != 1.0f) {
                llama_sampler_chain_add(sampler_, llama_sampler_init_penalties(
                    config_.repeat_last_n,
                    config_.repeat_penalty,
                    0.0f, 0.0f
                ));
            }
            
            uint32_t seed = config_.seed >= 0 ? static_cast<uint32_t>(config_.seed) : static_cast<uint32_t>(std::time(nullptr));
            llama_sampler_chain_add(sampler_, llama_sampler_init_dist(seed));
        }
        
        if (config_.debug_mode) {
            LEAFRA_DEBUG() << "Updated generation config and recreated sampler";
        }
    }
    
    GenerationStats get_last_stats() const {
        return last_stats_;
    }
    
    std::string get_model_info() const {
        if (!is_loaded()) {
            return "No model loaded";
        }
        
        std::ostringstream info;
        info << "Model Information:\n";
        info << "  - Path: " << config_.model_path << "\n";
        info << "  - Vocabulary size: " << vocab_size_ << "\n";
        info << "  - Context size: " << context_size_ << "\n";
        info << "  - Context used: " << context_used_ << "\n";
        info << "  - Embedding dimension: " << llama_model_n_embd(model_) << "\n";
        info << "  - Model size: " << llama_model_n_params(model_) << " parameters\n";
        info << "  - GPU layers: " << config_.n_gpu_layers << "\n";
        info << "  - Threads: " << config_.n_threads << "\n";
        
        return info.str();
    }
    
    std::string get_last_error() const {
        return last_error_;
    }
    
    bool supports_embeddings() const {
        return is_loaded() && llama_model_n_embd(model_) > 0;
    }
    
    bool set_system_prompt(const std::string& system_prompt) {
        if (!is_loaded()) {
            last_error_ = "Model not loaded";
            return false;
        }
        
        system_prompt_ = system_prompt;
        
        // Clear context and set system prompt if provided
        if (!system_prompt.empty()) {
            reset_context();
            
            auto tokens = tokenize(system_prompt, true);
            if (tokens.empty()) {
                last_error_ = "Failed to tokenize system prompt";
                return false;
            }
            
            // Add system prompt to context
            llama_batch batch = llama_batch_init(config_.n_batch, 0, 1);
            
            for (size_t i = 0; i < tokens.size(); i += config_.n_batch) {
                this->batch_clear(batch);
                
                size_t batch_size = std::min(static_cast<size_t>(config_.n_batch), tokens.size() - i);
                for (size_t j = 0; j < batch_size; ++j) {
                    this->batch_add(batch, tokens[i + j], static_cast<llama_pos>(i + j), {0}, j == batch_size - 1);
                }
                
                if (llama_decode(context_, batch) != 0) {
                    last_error_ = "Failed to evaluate system prompt";
                    llama_batch_free(batch);
                    return false;
                }
                
                context_used_ += static_cast<int32_t>(batch_size);
            }
            
            llama_batch_free(batch);
        }
        
        return true;
    }
    
    std::string generate_chat_response(const std::vector<ChatMessage>& messages, int32_t max_tokens) {
        std::string formatted_prompt = format_chat_prompt(messages, true);
        if (formatted_prompt.empty()) {
            last_error_ = "Failed to format chat prompt";
            return "";
        }
        
        return generate_text(formatted_prompt, max_tokens);
    }
    
    bool generate_chat_response_stream(const std::vector<ChatMessage>& messages, TokenCallback callback, int32_t max_tokens) {
        std::string formatted_prompt = format_chat_prompt(messages, true);
        if (formatted_prompt.empty()) {
            last_error_ = "Failed to format chat prompt";
            return false;
        }
        
        return generate_text_stream(formatted_prompt, callback, max_tokens);
    }
    
    std::string format_chat_prompt(const std::vector<ChatMessage>& messages, bool add_generation_prompt) {
        if (!is_loaded()) {
            last_error_ = "Model not loaded";
            return "";
        }
        
        // Convert ChatMessage to llama_chat_message
        std::vector<llama_chat_message> llama_messages;
        std::vector<std::string> role_storage, content_storage;
        
        // Store strings to keep them alive
        role_storage.reserve(messages.size());
        content_storage.reserve(messages.size());
        llama_messages.reserve(messages.size());
        
        for (const auto& msg : messages) {
            role_storage.push_back(msg.role);
            content_storage.push_back(msg.content);
            llama_messages.push_back({
                role_storage.back().c_str(),
                content_storage.back().c_str()
            });
        }
        
        // Use custom template if set, otherwise use model's default (nullptr)
        const char* template_name = chat_template_name_.empty() ? nullptr : chat_template_name_.c_str();
        
        // First pass to get required buffer size
        int32_t required_size = llama_chat_apply_template(
            template_name,
            llama_messages.data(),
            llama_messages.size(),
            add_generation_prompt,
            nullptr,
            0
        );
        
        if (required_size < 0) {
            last_error_ = "Chat template not supported or invalid";
            return "";
        }
        
        // Allocate buffer and format
        std::vector<char> buffer(required_size + 1);
        int32_t actual_size = llama_chat_apply_template(
            template_name,
            llama_messages.data(),
            llama_messages.size(),
            add_generation_prompt,
            buffer.data(),
            buffer.size()
        );
        
        if (actual_size < 0 || actual_size >= static_cast<int32_t>(buffer.size())) {
            last_error_ = "Failed to format chat template";
            return "";
        }
        
        return std::string(buffer.data(), actual_size);
    }
    
    bool set_chat_template(const std::string& template_name) {
        // Validate template by trying to format an empty conversation
        std::vector<llama_chat_message> test_messages = {
            {"user", "test"}
        };
        
        int32_t test_size = llama_chat_apply_template(
            template_name.c_str(),
            test_messages.data(),
            test_messages.size(),
            true,
            nullptr,
            0
        );
        
        if (test_size < 0) {
            last_error_ = "Invalid or unsupported chat template: " + template_name;
            return false;
        }
        
        chat_template_name_ = template_name;
        return true;
    }
    
    std::string get_chat_template() const {
        if (!chat_template_name_.empty()) {
            return chat_template_name_;
        }
        
        // Try to get model's default template
        if (is_loaded()) {
            const char* model_template = llama_model_chat_template(model_, nullptr);
            if (model_template) {
                return std::string(model_template);
            }
        }
        
        return "";
    }
    
private:
    // Helper methods
    std::string get_current_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        return std::ctime(&time_t);
    }
    
    void seed_rng(int32_t seed) {
        if (seed < 0) {
            seed = static_cast<int32_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        }
        g_rng.seed(static_cast<unsigned>(seed));
    }
    
    void batch_clear(llama_batch& batch) {
        batch.n_tokens = 0;
    }
    
    void batch_add(llama_batch& batch, llama_token id, llama_pos pos, const std::vector<llama_seq_id>& seq_ids, bool logits) {
        batch.token[batch.n_tokens] = id;
        batch.pos[batch.n_tokens] = pos;
        batch.n_seq_id[batch.n_tokens] = static_cast<int32_t>(seq_ids.size());
        for (size_t i = 0; i < seq_ids.size(); ++i) {
            batch.seq_id[batch.n_tokens][i] = seq_ids[i];
        }
        batch.logits[batch.n_tokens] = logits ? 1 : 0;
        
        batch.n_tokens++;
    }
    


    // Member variables
    llama_context* context_;
    llama_model* model_;
    const llama_vocab* vocab_;
    llama_sampler* sampler_;  // Modern sampling chain
    LlamaCppConfig config_;
    int32_t vocab_size_ = 0;
    int32_t context_size_ = 0;
    int32_t context_used_;
    std::string system_prompt_;
    GenerationStats last_stats_;
    mutable std::string last_error_;
    std::string chat_template_name_;  // Custom template name if set
};

// LlamaCppModel implementation
LlamaCppModel::LlamaCppModel() : pImpl(std::make_unique<Impl>()) {}

LlamaCppModel::~LlamaCppModel() = default;

LlamaCppModel::LlamaCppModel(LlamaCppModel&&) noexcept = default;

LlamaCppModel& LlamaCppModel::operator=(LlamaCppModel&&) noexcept = default;

bool LlamaCppModel::load_model(const LlamaCppConfig& config) {
    return pImpl->load_model(config);
}

bool LlamaCppModel::is_loaded() const {
    return pImpl->is_loaded();
}

void LlamaCppModel::unload() {
    pImpl->unload();
}

std::string LlamaCppModel::generate_text(const std::string& prompt, int32_t max_tokens) {
    return pImpl->generate_text(prompt, max_tokens);
}

bool LlamaCppModel::generate_text_stream(const std::string& prompt, TokenCallback callback, int32_t max_tokens) {
    return pImpl->generate_text_stream(prompt, callback, max_tokens);
}

std::string LlamaCppModel::continue_generation(const std::string& additional_prompt, int32_t max_tokens) {
    return pImpl->continue_generation(additional_prompt, max_tokens);
}

void LlamaCppModel::reset_context() {
    pImpl->reset_context();
}

std::vector<int32_t> LlamaCppModel::tokenize(const std::string& text, bool add_special) {
    return pImpl->tokenize(text, add_special);
}

std::string LlamaCppModel::detokenize(const std::vector<int32_t>& tokens) {
    return pImpl->detokenize(tokens);
}

std::string LlamaCppModel::get_token_text(int32_t token_id) {
    return pImpl->get_token_text(token_id);
}

double LlamaCppModel::calculate_perplexity(const std::string& text) {
    return pImpl->calculate_perplexity(text);
}

std::vector<float> LlamaCppModel::get_embeddings(const std::string& text) {
    return pImpl->get_embeddings(text);
}

int32_t LlamaCppModel::get_vocab_size() const {
    return pImpl->get_vocab_size();
}

int32_t LlamaCppModel::get_context_size() const {
    return pImpl->get_context_size();
}

int32_t LlamaCppModel::get_context_used() const {
    return pImpl->get_context_used();
}

const LlamaCppConfig& LlamaCppModel::get_config() const {
    return pImpl->get_config();
}

void LlamaCppModel::update_generation_config(const LlamaCppConfig& config) {
    pImpl->update_generation_config(config);
}

GenerationStats LlamaCppModel::get_last_stats() const {
    return pImpl->get_last_stats();
}

std::string LlamaCppModel::get_model_info() const {
    return pImpl->get_model_info();
}

std::string LlamaCppModel::get_last_error() const {
    return pImpl->get_last_error();
}

bool LlamaCppModel::supports_embeddings() const {
    return pImpl->supports_embeddings();
}

bool LlamaCppModel::set_system_prompt(const std::string& system_prompt) {
    return pImpl->set_system_prompt(system_prompt);
}

std::string LlamaCppModel::generate_chat_response(const std::vector<ChatMessage>& messages, int32_t max_tokens) {
    return pImpl->generate_chat_response(messages, max_tokens);
}

bool LlamaCppModel::generate_chat_response_stream(const std::vector<ChatMessage>& messages, TokenCallback callback, int32_t max_tokens) {
    return pImpl->generate_chat_response_stream(messages, callback, max_tokens);
}

std::string LlamaCppModel::format_chat_prompt(const std::vector<ChatMessage>& messages, bool add_generation_prompt) {
    return pImpl->format_chat_prompt(messages, add_generation_prompt);
}

bool LlamaCppModel::set_chat_template(const std::string& template_name) {
    return pImpl->set_chat_template(template_name);
}

std::string LlamaCppModel::get_chat_template() const {
    return pImpl->get_chat_template();
}

// Global functions
namespace global {

bool initialize(bool log_disable) {
    if (g_initialized) {
        return true;
    }
    
    llama_backend_init();
    
    if (log_disable) {
        llama_log_set([](enum ggml_log_level level, const char* text, void* user_data) {
            // Suppress all llama.cpp logging
        }, nullptr);
    }
    
    g_initialized = true;
    
    LEAFRA_INFO() << "✅ LlamaCpp backend initialized";
    LEAFRA_INFO() << "  - Version: " << get_version();
    LEAFRA_INFO() << "  - Logging: " << (log_disable ? "disabled" : "enabled");
    
    return true;
}

void cleanup() {
    if (g_initialized) {
        llama_backend_free();
        g_initialized = false;
        LEAFRA_INFO() << "🧹 LlamaCpp backend cleaned up";
    }
}

bool is_initialized() {
    return g_initialized;
}

std::string get_version() {
    return "llama.cpp integration v1.0";
}

std::string get_system_info() {
    return "LlamaCpp system info not available";
}

} // namespace global

// Utility functions
namespace utils {

bool is_valid_model_file(const std::string& model_path) {
    std::ifstream file(model_path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // Check for GGUF magic number
    char magic[4];
    file.read(magic, 4);
    
    // GGUF magic is "GGUF"
    return file.good() && 
           magic[0] == 'G' && magic[1] == 'G' && 
           magic[2] == 'U' && magic[3] == 'F';
}

LlamaCppConfig get_recommended_config(const std::string& model_path) {
    LlamaCppConfig config(model_path);
    
    // Try to analyze model and set reasonable defaults
    if (is_valid_model_file(model_path)) {
        // Basic defaults - could be enhanced with actual model analysis
        config.n_ctx = 2048;
        config.n_batch = 512;
        config.temperature = 0.7f;
        config.top_p = 0.9f;
        config.top_k = 40;
        
        // Set thread count based on available cores
        config.n_threads = static_cast<int32_t>(std::thread::hardware_concurrency());
        config.n_threads_batch = config.n_threads;
        
        LEAFRA_INFO() << "Generated recommended config for: " << model_path;
    } else {
        LEAFRA_WARNING() << "Could not analyze model file, using defaults: " << model_path;
    }
    
    return config;
}



LlamaCppConfig from_llm_config(const LLMConfig& llm_config) {
    LlamaCppConfig config(llm_config.model_path);
    
    // Map basic parameters
    config.n_ctx = llm_config.context_size;
    config.n_predict = llm_config.max_tokens;
    config.n_batch = llm_config.batch_size;
    config.n_threads = llm_config.num_threads;
    config.n_threads_batch = llm_config.num_threads;
    
    // Map generation parameters
    config.temperature = llm_config.temperature;
    config.top_p = llm_config.top_p;
    config.top_k = llm_config.top_k;
    config.repeat_penalty = llm_config.repetition_penalty;
    config.repeat_last_n = llm_config.repetition_context;
    
    // Map hardware/performance parameters
    config.n_gpu_layers = llm_config.gpu_layers;
    config.use_mmap = llm_config.use_memory_mapping;
    config.use_mlock = llm_config.use_memory_locking;
    
    // Map system configuration
    config.seed = llm_config.seed;
    config.debug_mode = llm_config.debug_mode;
    
    return config;
}

std::vector<std::string> get_available_chat_templates() {
    std::vector<std::string> templates;
    
    // Get the number of built-in templates
    const char* template_names[64]; // Should be enough for all built-in templates
    int32_t count = llama_chat_builtin_templates(template_names, 64);
    
    if (count > 0) {
        templates.reserve(count);
        for (int32_t i = 0; i < count; ++i) {
            if (template_names[i]) {
                templates.emplace_back(template_names[i]);
            }
        }
    }
    
    return templates;
}

} // namespace utils

} // namespace llamacpp
} // namespace leafra

#endif // LEAFRA_HAS_LLAMACPP