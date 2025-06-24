# Chat Templates Quick Reference

A quick reference for using chat templates in LeafraSDK.

## Basic Usage

```cpp
#include "leafra/leafra_llamacpp.h"
using namespace leafra::llamacpp;

// Initialize and load model
LlamaCppModel model;
LlamaCppConfig config("model.gguf");
model.load_model(config);

// Create conversation
std::vector<ChatMessage> messages = {
    {"system", "You are a helpful assistant."},
    {"user", "Hello!"}
};

// Generate response
std::string response = model.generate_chat_response(messages, 150);
```

## Available Templates

| Template | Description | Example Format |
|----------|-------------|----------------|
| `chatml` | OpenAI ChatML format | `<\|im_start\|>user\nHello<\|im_end\|>` |
| `llama2` | Llama 2 format | `<s>[INST] Hello [/INST]` |
| `llama3` | Llama 3 format | `<\|start_header_id\|>user<\|end_header_id\|>Hello` |
| `mistral-v1` | Mistral v1 format | `[INST] Hello [/INST]` |
| `mistral-v3` | Mistral v3 format | `[INST] Hello [/INST]` |
| `mistral-v7` | Mistral v7 format | `[INST] Hello [/INST]` |
| `zephyr` | Zephyr format | `<\|user\|>Hello<\|assistant\|>` |
| `gemma` | Gemma format | `<start_of_turn>user\nHello<end_of_turn>` |
| `vicuna` | Vicuna format | `USER: Hello\nASSISTANT:` |
| `phi3` | Phi-3 format | Custom Microsoft format |
| `openchat` | OpenChat format | ChatML variant |

## Template Management

```cpp
// List available templates
auto templates = utils::get_available_chat_templates();
for (const auto& tmpl : templates) {
    std::cout << tmpl << std::endl;
}

// Set specific template
model.set_chat_template("chatml");

// Get current template
std::string current = model.get_chat_template();

// Auto-detect (default behavior)
// Uses model's built-in template or falls back to chatml
```

## Message Roles

| Role | Purpose | Example |
|------|---------|---------|
| `system` | Set AI behavior/personality | `"You are a helpful coding assistant."` |
| `user` | Human input/questions | `"How do I sort an array in Python?"` |
| `assistant` | AI responses | `"You can use the sorted() function..."` |

## Common Patterns

### Simple Q&A
```cpp
std::vector<ChatMessage> qa = {
    {"user", "What is the capital of France?"}
};
std::string answer = model.generate_chat_response(qa);
```

### With System Prompt
```cpp
std::vector<ChatMessage> conversation = {
    {"system", "You are a Python expert. Provide concise code examples."},
    {"user", "How to read a file?"}
};
std::string code = model.generate_chat_response(conversation);
```

### Multi-turn Conversation
```cpp
std::vector<ChatMessage> chat = {
    {"user", "What's 2+2?"},
    {"assistant", "2+2 equals 4."},
    {"user", "What about 2+3?"}
};
std::string response = model.generate_chat_response(chat);
```

### Streaming Chat
```cpp
model.generate_chat_response_stream(messages,
    [](const std::string& token, bool is_final) {
        std::cout << token << std::flush;
        return true; // continue
    }
);
```

## Template Detection

LeafraSDK automatically detects the appropriate template based on:

1. **Model metadata** (if available in GGUF)
2. **Model filename** patterns:
   - `*llama-3*` → llama3
   - `*llama-2*` → llama2
   - `*mistral*` → mistral-v1
   - `*zephyr*` → zephyr
   - `*gemma*` → gemma
   - etc.
3. **Manual override** via `set_chat_template()`
4. **Default fallback** to chatml

## Error Handling

```cpp
// Check template validity
if (!model.set_chat_template("custom_template")) {
    std::cerr << "Error: " << model.get_last_error() << std::endl;
}

// Check generation success
std::string response = model.generate_chat_response(messages);
if (response.empty()) {
    std::cerr << "Generation failed: " << model.get_last_error() << std::endl;
}
```

## Manual Formatting

```cpp
// Format without generation
std::string formatted = model.format_chat_prompt(messages, true);
std::cout << "Formatted prompt:\n" << formatted << std::endl;

// Use with regular text generation
std::string response = model.generate_text(formatted, 100);
```

## Best Practices

1. **Always include system prompts** for better behavior control
2. **Use appropriate templates** for your model family
3. **Handle context limits** in long conversations
4. **Check for errors** after template operations
5. **Let auto-detection work** unless you need specific formatting

## Context Management in Conversations

```cpp
class ChatManager {
    std::vector<ChatMessage> conversation_;
    
    void add_message(const std::string& role, const std::string& content) {
        conversation_.push_back({role, content});
        
        // Trim if too long (keep system + recent messages)
        if (conversation_.size() > 20) {
            std::vector<ChatMessage> trimmed;
            if (!conversation_.empty() && conversation_[0].role == "system") {
                trimmed.push_back(conversation_[0]); // Keep system prompt
            }
            trimmed.insert(trimmed.end(), 
                          conversation_.end() - 15, 
                          conversation_.end());
            conversation_ = std::move(trimmed);
        }
    }
};
```

## Template Compatibility

| Model Family | Recommended Template | Auto-Detected |
|--------------|---------------------|---------------|
| Llama 3.x | `llama3` | ✅ |
| Llama 2.x | `llama2` | ✅ |
| Mistral 7B | `mistral-v1` | ✅ |
| Mistral Nemo | `mistral-v7` | ✅ |
| Zephyr | `zephyr` | ✅ |
| Gemma | `gemma` | ✅ |
| CodeLlama | `llama2` | ✅ |
| Vicuna | `vicuna` | ✅ |
| OpenChat | `openchat` | ✅ |
| ChatGLM | `chatglm-3` | ✅ |
| Phi-3 | `phi3` | ✅ |
| Custom Models | `chatml` (fallback) | ❌ |

---

For more detailed documentation, see [LLM_GUIDE.md](LLM_GUIDE.md). 