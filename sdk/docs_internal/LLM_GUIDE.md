# LeafraSDK LLM Guide

A comprehensive guide to using Large Language Models (LLMs) with LeafraSDK, including text generation, chat conversations, and advanced features.

---

## Documentation Overview

This is the main LLM documentation for LeafraSDK. For additional resources:
- **[CHAT_TEMPLATES_QUICK_REFERENCE.md](CHAT_TEMPLATES_QUICK_REFERENCE.md)** - Quick reference for chat template functionality

### Chat Template Implementation

LeafraSDK includes comprehensive chat template support:

#### ✅ Features
- **Auto-detection** of chat templates from model metadata and filename
- **40+ built-in templates** including Llama, Mistral, Gemma, Zephyr, ChatML, etc.
- **Streaming support** for real-time conversation interfaces
- **Context management** for multi-turn conversations
- **Error handling** with detailed error messages
- **Manual override** for custom template requirements

#### 🔧 Technical Implementation
- Uses llama.cpp's native `llama_chat_apply_template()` function
- Zero-copy string handling for performance
- Memory-safe implementation with proper lifetime management
- Compatible with all existing LeafraSDK features

#### 📖 Quick Usage
```cpp
#include "leafra/leafra_llamacpp.h"
using namespace leafra::llamacpp;

// Load model and auto-detect template
LlamaCppModel model;
model.load_model(config);

// Create conversation
std::vector<ChatMessage> messages = {
    {"system", "You are a helpful AI assistant."},
    {"user", "Hello! How can you help me today?"}
};

// Generate response with proper template formatting
std::string response = model.generate_chat_response(messages, 150);
```

#### 🏗️ Architecture Notes

**LlamaCpp Integration:**
- **Backend**: Uses prebuilt llama.cpp framework for optimal performance
- **Models**: Supports GGUF format with quantization (Q4_K_M, Q5_K_M, Q8_0, etc.)
- **Platforms**: iOS, macOS, Android, Windows, Linux
- **Acceleration**: Metal (Apple), CUDA (NVIDIA), CPU fallback

**Chat Template System:**
- **Detection**: Automatic template detection from model metadata and filename patterns
- **Fallback**: Graceful fallback to ChatML format for unknown models
- **Validation**: Template validation before application to prevent errors
- **Extensibility**: Easy to add new templates as they become available

---

## Table of Contents

- [Overview](#overview)
- [Quick Start](#quick-start)
- [LLM Backends](#llm-backends)
- [Basic Usage](#basic-usage)
- [Chat Templates](#chat-templates)
- [Advanced Features](#advanced-features)
- [Configuration](#configuration)
- [Best Practices](#best-practices)
- [Troubleshooting](#troubleshooting)
- [Examples](#examples)

## Overview

LeafraSDK provides powerful LLM integration through the LlamaCpp backend, offering high-performance GGUF model inference with comprehensive features including chat templates, streaming generation, embeddings, and advanced sampling controls.

## Quick Start

### 1. Initialize the SDK

```cpp
#include "leafra/leafra_core.h"
#include "leafra/leafra_llamacpp.h"

// Initialize the LlamaCpp backend
if (!leafra::llamacpp::global::initialize()) {
    std::cerr << "Failed to initialize LlamaCpp" << std::endl;
    return -1;
}
```

### 2. Configure and Load a Model

```cpp
using namespace leafra::llamacpp;

// Configure the model
LlamaCppConfig config("path/to/your/model.gguf");
config.n_ctx = 4096;        // Context size
config.n_gpu_layers = 32;   // GPU acceleration
config.temperature = 0.7f;  // Creativity level

// Create and load model
LlamaCppModel model;
if (!model.load_model(config)) {
    std::cerr << "Failed to load model: " << model.get_last_error() << std::endl;
    return -1;
}
```

### 3. Generate Text

```cpp
// Simple text generation
std::string prompt = "Write a short story about a robot:";
std::string response = model.generate_text(prompt, 200);
std::cout << response << std::endl;
```

## LLM Backends

### LlamaCpp

**Features**: High-performance GGUF model inference with comprehensive LLM capabilities

```cpp
#include "leafra/leafra_llamacpp.h"
using namespace leafra::llamacpp;

LlamaCppModel model;
LlamaCppConfig config("model.gguf");
model.load_model(config);
```

**Capabilities**:
- ✅ GGUF model support
- ✅ GPU acceleration (Metal/CUDA)
- ✅ Chat templates
- ✅ Streaming generation
- ✅ Embeddings
- ✅ Context management
- ✅ Advanced sampling

**Supported Models**: Llama, Mistral, Gemma, Qwen, CodeLlama, and many more

## Basic Usage

### Text Generation

#### Synchronous Generation

```cpp
// Basic generation
std::string response = model.generate_text("Hello, world!", 100);

// With custom parameters
LlamaCppConfig config = model.get_config();
config.temperature = 0.3f;  // More focused
config.top_p = 0.9f;        // Nucleus sampling
model.update_generation_config(config);

std::string response = model.generate_text(prompt, 200);
```

#### Streaming Generation

```cpp
bool success = model.generate_text_stream(prompt, 
    [](const std::string& token, bool is_final) {
        if (!is_final) {
            std::cout << token << std::flush;
        } else {
            std::cout << "\n[Generation complete]" << std::endl;
        }
        return true; // Continue generation
    }, 
    200 // max tokens
);
```

#### User-Controlled Generation

```cpp
std::atomic<bool> should_stop(false);

// Start generation in background
std::thread generation_thread([&]() {
    model.generate_text_stream(prompt, 
        [&should_stop](const std::string& token, bool is_final) {
            if (should_stop.load()) {
                return false; // Stop generation
            }
            std::cout << token << std::flush;
            return true;
        }, 
        500
    );
});

// User can stop generation
std::string input;
std::getline(std::cin, input);
if (input == "stop") {
    should_stop = true;
}

generation_thread.join();
```

## Chat Templates

LeafraSDK supports automatic chat template formatting for conversational AI. This ensures proper formatting for different model families.

### Basic Chat Usage

```cpp
#include "leafra/leafra_llamacpp.h"
using namespace leafra::llamacpp;

// Create conversation
std::vector<ChatMessage> messages = {
    {"system", "You are a helpful AI assistant."},
    {"user", "What's the capital of France?"},
};

// Generate response (automatically formatted)
std::string response = model.generate_chat_response(messages, 150);
std::cout << "Assistant: " << response << std::endl;

// Add assistant response to conversation
messages.push_back({"assistant", response});
messages.push_back({"user", "What's the population?"});

// Continue conversation
response = model.generate_chat_response(messages, 100);
```

### Streaming Chat

```cpp
std::vector<ChatMessage> conversation = {
    {"system", "You are a coding assistant."},
    {"user", "Write a Python function to calculate fibonacci numbers."}
};

std::cout << "Assistant: ";
model.generate_chat_response_stream(conversation,
    [](const std::string& token, bool is_final) {
        if (!is_final) {
            std::cout << token << std::flush;
        }
        return true;
    },
    300
);
std::cout << std::endl;
```

### Supported Chat Templates

LeafraSDK automatically detects and applies the correct chat template based on the model. Supported templates include:

- **ChatML** (OpenAI-style): `<|im_start|>user\n...<|im_end|>`
- **Llama 2**: `<s>[INST] <<SYS>>...`
- **Llama 3**: `<|start_header_id|>user<|end_header_id|>...`
- **Mistral**: `[INST]...[/INST]`
- **Zephyr**: `<|user|>...<|assistant|>`
- **Gemma**: `<start_of_turn>user...`
- **Vicuna**: `USER: ... ASSISTANT:`
- **And many more...**

### Custom Chat Templates

```cpp
// List available templates
auto templates = utils::get_available_chat_templates();
for (const auto& tmpl : templates) {
    std::cout << "Available: " << tmpl << std::endl;
}

// Set specific template
if (model.set_chat_template("chatml")) {
    std::cout << "Using ChatML format" << std::endl;
} else {
    std::cout << "Template not supported: " << model.get_last_error() << std::endl;
}

// Get current template
std::string current = model.get_chat_template();
std::cout << "Current template: " << current << std::endl;
```

### Manual Template Formatting

```cpp
// Format messages without generation
std::vector<ChatMessage> messages = {
    {"user", "Hello!"},
    {"assistant", "Hi there!"},
    {"user", "How are you?"}
};

std::string formatted = model.format_chat_prompt(messages, true);
std::cout << "Formatted prompt:\n" << formatted << std::endl;

// Use with regular text generation
std::string response = model.generate_text(formatted, 100);
```

## Advanced Features

### Embeddings

```cpp
// Generate embeddings for text
std::vector<float> embeddings = model.get_embeddings("Hello, world!");
std::cout << "Embedding dimensions: " << embeddings.size() << std::endl;

// Use for semantic search, clustering, etc.
```

### Context Management

```cpp
// Check context usage
std::cout << "Context used: " << model.get_context_used() 
          << "/" << model.get_context_size() << std::endl;

// Reset context when needed
model.reset_context();

// Continue generation with additional context
std::string continuation = model.continue_generation("And then", 50);
```

### System Prompts

```cpp
// Set system behavior
model.set_system_prompt("You are a creative writing assistant. "
                       "Write engaging stories with vivid descriptions.");

// System prompt is automatically included in chat responses
std::vector<ChatMessage> messages = {
    {"user", "Write a short story about a magical forest."}
};
std::string story = model.generate_chat_response(messages, 300);
```

### Performance Monitoring

```cpp
// Get generation statistics
GenerationStats stats = model.get_last_stats();
std::cout << "Tokens generated: " << stats.tokens_generated << std::endl;
std::cout << "Generation time: " << stats.generation_time_ms << "ms" << std::endl;
std::cout << "Tokens/second: " << stats.tokens_per_second << std::endl;

// Model information
std::cout << "Model info: " << model.get_model_info() << std::endl;
std::cout << "Supports embeddings: " << model.supports_embeddings() << std::endl;
```

## Configuration

### Basic Configuration

```cpp
LlamaCppConfig config("model.gguf");

// Context and batch settings
config.n_ctx = 4096;           // Context window
config.n_batch = 512;          // Batch size for processing
config.n_predict = 256;        // Default max tokens to generate

// Generation parameters
config.temperature = 0.7f;     // Randomness (0.0 = deterministic, 1.0+ = creative)
config.top_p = 0.9f;          // Nucleus sampling
config.top_k = 40;            // Top-k sampling
config.repeat_penalty = 1.1f;  // Repetition penalty

// Performance settings
config.n_threads = 8;          // CPU threads
config.n_gpu_layers = 32;      // GPU layers (0 = CPU only)
config.use_mmap = true;        // Memory mapping
config.use_mlock = false;      // Memory locking
```

### Advanced Configuration

```cpp
// Fine-tune sampling
config.repeat_last_n = 64;     // Look-back for repetition penalty
config.frequency_penalty = 0.0f;
config.presence_penalty = 0.0f;
config.mirostat = 0;           // Mirostat sampling
config.mirostat_tau = 5.0f;
config.mirostat_eta = 0.1f;

// Debug and logging
config.debug_mode = true;      // Verbose output
config.seed = 42;              // Reproducible results

// Load model with config
model.load_model(config);
```

### Runtime Configuration Updates

```cpp
// Update generation parameters without reloading
LlamaCppConfig new_config = model.get_config();
new_config.temperature = 0.3f;  // More focused
new_config.top_p = 0.8f;       // Less diverse
model.update_generation_config(new_config);
```

## Best Practices

### Model Selection

1. **Choose appropriate model size**:
   - 7B models: Good balance of speed and quality
   - 13B models: Better quality, slower inference
   - 30B+ models: Highest quality, requires significant resources

2. **Quantization levels**:
   - Q4_K_M: Good balance (recommended)
   - Q5_K_M: Better quality, larger size
   - Q8_0: Highest quality, largest size

### Performance Optimization

```cpp
// Optimize for your hardware
LlamaCppConfig config("model.gguf");

// GPU acceleration (if available)
config.n_gpu_layers = -1;  // Use all GPU layers

// CPU optimization
config.n_threads = std::thread::hardware_concurrency();

// Memory optimization
config.use_mmap = true;     // Enable memory mapping
config.n_batch = 1024;      // Larger batches for throughput
```

### Memory Management

```cpp
// Monitor memory usage
if (model.get_context_used() > model.get_context_size() * 0.8) {
    // Approaching context limit
    model.reset_context();
    // Or implement sliding window context
}

// Unload model when done
model.unload();
```

### Error Handling

```cpp
// Always check for errors
if (!model.load_model(config)) {
    std::cerr << "Load failed: " << model.get_last_error() << std::endl;
    return -1;
}

// Check generation results
std::string response = model.generate_text(prompt, 100);
if (response.empty()) {
    std::cerr << "Generation failed: " << model.get_last_error() << std::endl;
}
```

## Troubleshooting

### Common Issues

#### Model Loading Failed

```cpp
// Check file exists and permissions
if (!utils::is_valid_model_file(model_path)) {
    std::cerr << "Invalid model file: " << model_path << std::endl;
}

// Try recommended config
LlamaCppConfig config = utils::get_recommended_config(model_path);
```

#### Out of Memory

```cpp
// Reduce GPU layers
config.n_gpu_layers = 16;  // Instead of -1

// Reduce context size
config.n_ctx = 2048;  // Instead of 4096

// Enable memory mapping
config.use_mmap = true;
```

#### Slow Generation

```cpp
// Increase batch size
config.n_batch = 1024;

// Use more GPU layers
config.n_gpu_layers = -1;

// Optimize threads
config.n_threads = std::thread::hardware_concurrency();
```

#### Poor Quality Output

```cpp
// Adjust temperature
config.temperature = 0.7f;  // Default, try 0.3-1.0 range

// Use nucleus sampling
config.top_p = 0.9f;

// Enable repetition penalty
config.repeat_penalty = 1.1f;
```

### Debug Mode

```cpp
// Enable detailed logging
config.debug_mode = true;
model.load_model(config);

// Check model information
std::cout << model.get_model_info() << std::endl;
std::cout << "Vocab size: " << model.get_vocab_size() << std::endl;
```

## Examples

### Complete Chat Application

```cpp
#include "leafra/leafra_llamacpp.h"
#include <iostream>
#include <vector>

using namespace leafra::llamacpp;

int main() {
    // Initialize model
    LlamaCppModel model;
    LlamaCppConfig config("llama-3.2-3b-instruct.gguf");
    config.n_ctx = 2048;
    config.temperature = 0.7f;
    
    if (!model.load_model(config)) {
        std::cerr << "Failed to load model: " << model.get_last_error() << std::endl;
        return 1;
    }
    
    // Set up conversation
    std::vector<ChatMessage> conversation = {
        {"system", "You are a helpful programming assistant."}
    };
    
    std::string user_input;
    std::cout << "Chat with AI (type 'quit' to exit):\n";
    
    while (true) {
        std::cout << "\nYou: ";
        std::getline(std::cin, user_input);
        
        if (user_input == "quit") break;
        
        // Add user message
        conversation.emplace_back("user", user_input);
        
        // Generate response
        std::cout << "Assistant: ";
        
        bool success = model.generate_chat_response_stream(
            conversation,
            [](const std::string& token, bool is_final) {
                std::cout << token << std::flush;
                return true; // Continue generation
            },
            200 // max tokens
        );
        
        if (success) {
            // Add assistant response to conversation
            // Note: In real implementation, collect tokens from callback
            conversation.emplace_back("assistant", "[response collected from callback]");
        } else {
            std::cout << "Error: " << model.get_last_error() << std::endl;
        }
    }
    
    return 0;
}

---

## Additional Resources

- [LeafraSDK API Reference](../docs/API_REFERENCE.md)
- [Model Compatibility Guide](../docs/MODEL_COMPATIBILITY.md)
- [Performance Optimization](../docs/PERFORMANCE.md)
- [LlamaCpp Documentation](https://github.com/ggerganov/llama.cpp)

For technical support or questions, please refer to the project documentation or contact the development team. 