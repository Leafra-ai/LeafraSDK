#include "leafra_llamacpp.h"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <cassert>
#include <memory>

using namespace leafra::llamacpp;

// Simple test assertion macros
#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        std::cerr << "ASSERTION FAILED: " << #condition << " at line " << __LINE__ << std::endl; \
        return false; \
    }

#define ASSERT_FALSE(condition) \
    if (condition) { \
        std::cerr << "ASSERTION FAILED: " << #condition << " should be false at line " << __LINE__ << std::endl; \
        return false; \
    }

#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        std::cerr << "ASSERTION FAILED: Expected " << (expected) << " but got " << (actual) << " at line " << __LINE__ << std::endl; \
        return false; \
    }

#define ASSERT_GT(value, threshold) \
    if ((value) <= (threshold)) { \
        std::cerr << "ASSERTION FAILED: " << (value) << " should be > " << (threshold) << " at line " << __LINE__ << std::endl; \
        return false; \
    }

#define ASSERT_GE(value, threshold) \
    if ((value) < (threshold)) { \
        std::cerr << "ASSERTION FAILED: " << (value) << " should be >= " << (threshold) << " at line " << __LINE__ << std::endl; \
        return false; \
    }

#define ASSERT_LE(value, threshold) \
    if ((value) > (threshold)) { \
        std::cerr << "ASSERTION FAILED: " << (value) << " should be <= " << (threshold) << " at line " << __LINE__ << std::endl; \
        return false; \
    }

class LlamaCppTester {
private:
    std::string model_path_;
    LlamaCppConfig config_;
    std::unique_ptr<LlamaCppModel> model_;
    
public:
    LlamaCppTester() {
        // Model path - using relative path from the build directory
        model_path_ = "../../../third_party/models/llm/unsloth/Llama-3.2-3B-Instruct-Q4_K_M.gguf";
        
        // Default configuration
        config_ = LlamaCppConfig(model_path_);
        config_.n_ctx = 2048;
        config_.n_predict = 50;
        config_.temperature = 0.7f;
        config_.top_p = 0.9f;
        config_.top_k = 40;
        config_.n_threads = 4;
        config_.debug_mode = false;
        
        model_ = std::make_unique<LlamaCppModel>();
    }
    
    ~LlamaCppTester() {
        if (model_) {
            model_->unload();
        }
    }
    
    bool test_model_loading_and_state() {
        std::cout << "Testing model loading and state..." << std::endl;
        
        // Initially model should not be loaded
        ASSERT_FALSE(model_->is_loaded());
        
        // Load model
        if (!model_->load_model(config_)) {
            std::cerr << "Failed to load model: " << model_->get_last_error() << std::endl;
            return false;
        }
        
        // Model should now be loaded
        ASSERT_TRUE(model_->is_loaded());
        
        // Check basic model info
        ASSERT_GT(model_->get_vocab_size(), 0);
        ASSERT_GT(model_->get_context_size(), 0);
        ASSERT_EQ(model_->get_context_used(), 0);
        
        // Get model info should return non-empty string
        std::string model_info = model_->get_model_info();
        ASSERT_FALSE(model_info.empty());
        
        // No error should be present after successful loading
        ASSERT_TRUE(model_->get_last_error().empty());
        
        std::cout << "✅ Model loading and state test passed" << std::endl;
        return true;
    }
    
    bool test_tokenization_and_detokenization() {
        std::cout << "Testing tokenization and detokenization..." << std::endl;
        
        if (!model_->is_loaded()) {
            ASSERT_TRUE(model_->load_model(config_));
        }
        
        std::string test_text = "Hello, world! This is a test.";
        
        // Test tokenization
        auto tokens = model_->tokenize(test_text, false);
        ASSERT_FALSE(tokens.empty());
        ASSERT_GT(tokens.size(), 0);
        
        // Test tokenization with special tokens
        auto tokens_special = model_->tokenize(test_text, true);
        ASSERT_GE(tokens_special.size(), tokens.size());
        
        // Test detokenization
        std::string detokenized = model_->detokenize(tokens);
        ASSERT_FALSE(detokenized.empty());
        
        // Test individual token text
        if (!tokens.empty()) {
            std::string token_text = model_->get_token_text(tokens[0]);
            ASSERT_FALSE(token_text.empty());
        }
        
        std::cout << "✅ Tokenization and detokenization test passed" << std::endl;
        return true;
    }
    
    bool test_basic_text_generation() {
        std::cout << "Testing basic text generation..." << std::endl;
        
        if (!model_->is_loaded()) {
            ASSERT_TRUE(model_->load_model(config_));
        }
        
        std::string prompt = "The capital of France is";
        
        // Test synchronous generation
        std::string response = model_->generate_text(prompt, 20);
        ASSERT_FALSE(response.empty());
        
        // Check that context was used
        ASSERT_GT(model_->get_context_used(), 0);
        
        // Check generation stats
        auto stats = model_->get_last_stats();
        ASSERT_GT(stats.prompt_tokens, 0);
        ASSERT_GT(stats.generated_tokens, 0);
        ASSERT_GT(stats.tokens_per_second, 0.0);
        
        std::cout << "✅ Basic text generation test passed" << std::endl;
        std::cout << "   Generated: " << response << std::endl;
        return true;
    }
    
    bool test_streaming_text_generation() {
        std::cout << "Testing streaming text generation..." << std::endl;
        
        if (!model_->is_loaded()) {
            ASSERT_TRUE(model_->load_model(config_));
        }
        
        std::string prompt = "Once upon a time";
        std::string collected_response;
        int token_count = 0;
        bool final_called = false;
        
        // Test streaming generation
        bool success = model_->generate_text_stream(prompt, 
            [&](const std::string& token, bool is_final) {
                if (is_final) {
                    final_called = true;
                } else {
                    collected_response += token;
                    token_count++;
                }
                return true; // Continue generation
            }, 15);
        
        ASSERT_TRUE(success);
        ASSERT_FALSE(collected_response.empty());
        ASSERT_GT(token_count, 0);
        ASSERT_TRUE(final_called);
        
        std::cout << "✅ Streaming text generation test passed" << std::endl;
        std::cout << "   Generated: " << collected_response << std::endl;
        return true;
    }
    
    bool test_chat_template_functionality() {
        std::cout << "Testing chat template functionality..." << std::endl;
        
        if (!model_->is_loaded()) {
            ASSERT_TRUE(model_->load_model(config_));
        }
        
        // Test chat message creation
        std::vector<ChatMessage> messages = {
            {"system", "You are a helpful assistant."},
            {"user", "What is 2+2?"}
        };
        
        // Test chat prompt formatting
        std::string formatted_prompt = model_->format_chat_prompt(messages, true);
        ASSERT_FALSE(formatted_prompt.empty());
        
        // Test chat response generation
        std::string chat_response = model_->generate_chat_response(messages, 30);
        ASSERT_FALSE(chat_response.empty());
        
        std::cout << "✅ Chat template functionality test passed" << std::endl;
        std::cout << "   Chat response: " << chat_response << std::endl;
        return true;
    }
    
    bool test_context_management() {
        std::cout << "Testing context management..." << std::endl;
        
        if (!model_->is_loaded()) {
            ASSERT_TRUE(model_->load_model(config_));
        }
        
        // Initially context should be empty
        ASSERT_EQ(model_->get_context_used(), 0);
        
        // Generate some text to use context
        std::string prompt = "Hello world";
        model_->generate_text(prompt, 10);
        
        // Context should now be used
        int context_used = model_->get_context_used();
        ASSERT_GT(context_used, 0);
        
        // Test context reset
        model_->reset_context();
        ASSERT_EQ(model_->get_context_used(), 0);
        
        std::cout << "✅ Context management test passed" << std::endl;
        return true;
    }
    
    bool test_configuration_access() {
        std::cout << "Testing configuration access..." << std::endl;
        
        if (!model_->is_loaded()) {
            ASSERT_TRUE(model_->load_model(config_));
        }
        
        // Test config access
        const auto& retrieved_config = model_->get_config();
        ASSERT_EQ(retrieved_config.model_path, config_.model_path);
        ASSERT_EQ(retrieved_config.n_ctx, config_.n_ctx);
        ASSERT_EQ(retrieved_config.temperature, config_.temperature);
        
        std::cout << "✅ Configuration access test passed" << std::endl;
        return true;
    }
    
    bool test_error_handling() {
        std::cout << "Testing error handling..." << std::endl;
        
        // Create a fresh model for error testing
        auto error_model = std::make_unique<LlamaCppModel>();
        
        // Test loading non-existent model
        LlamaCppConfig bad_config("non_existent_model.gguf");
        bool load_success = error_model->load_model(bad_config);
        ASSERT_FALSE(load_success);
        
        // Should have error message
        std::string error = error_model->get_last_error();
        ASSERT_FALSE(error.empty());
        
        // Model should not be loaded
        ASSERT_FALSE(error_model->is_loaded());
        
        // Operations on unloaded model should fail gracefully
        std::string response = error_model->generate_text("test", 10);
        ASSERT_TRUE(response.empty());
        
        auto tokens = error_model->tokenize("test", false);
        ASSERT_TRUE(tokens.empty());
        
        std::cout << "✅ Error handling test passed" << std::endl;
        return true;
    }
    
    bool run_all_tests() {
        std::cout << "=== LlamaCpp Model Unit Tests ===" << std::endl;
        
        int passed = 0;
        int total = 0;
        
        // Run all tests
        total++; if (test_model_loading_and_state()) passed++;
        total++; if (test_tokenization_and_detokenization()) passed++;
        total++; if (test_basic_text_generation()) passed++;
        total++; if (test_streaming_text_generation()) passed++;
        total++; if (test_chat_template_functionality()) passed++;
        total++; if (test_context_management()) passed++;
        total++; if (test_configuration_access()) passed++;
        total++; if (test_error_handling()) passed++;
        
        std::cout << "\n=== Test Results ===" << std::endl;
        std::cout << "Passed: " << passed << "/" << total << std::endl;
        
        if (passed == total) {
            std::cout << "🎉 All tests passed!" << std::endl;
            return true;
        } else {
            std::cout << "❌ " << (total - passed) << " test(s) failed" << std::endl;
            return false;
        }
    }
};

int main() {
    // Initialize LlamaCpp library
    if (!global::initialize(true)) {
        std::cerr << "Failed to initialize LlamaCpp library" << std::endl;
        return 1;
    }
    
    // Run tests
    LlamaCppTester tester;
    bool success = tester.run_all_tests();
    
    // Cleanup LlamaCpp library
    global::cleanup();
    
    return success ? 0 : 1;
} 