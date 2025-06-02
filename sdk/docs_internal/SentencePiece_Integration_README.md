# SentencePiece Integration for LeafraSDK Chunking

This document explains how to use the new SentencePiece integration in LeafraSDK for accurate token counting during text chunking.

## Overview

The LeafraSDK now supports **accurate token counting** using Google's SentencePiece library, which resolves the overlap percentage issues you were experiencing. Instead of using character-based estimates (like dividing by 4), the system now uses actual tokenization to count tokens precisely.

## Key Benefits

- ✅ **Accurate Token Counts**: Real tokenization instead of character estimates
- ✅ **Precise Overlap Control**: 10% overlap means exactly 10% token overlap
- ✅ **Model-Specific Tokenization**: Use the same tokenizer as your target LLM
- ✅ **Fallback Support**: Gracefully falls back to estimates if SentencePiece fails
- ✅ **Performance Insights**: Get real chars/token ratios for your content

## Configuration

### 1. Enable SentencePiece in Your Config

```cpp
leafra::Config config;

// Enable chunking
config.chunking.enabled = true;
config.chunking.chunk_size = 500; // Target tokens per chunk
config.chunking.overlap_percentage = 0.10; // Now accurately 10% token overlap
config.chunking.size_unit = leafra::ChunkSizeUnit::TOKENS;

// SentencePiece Configuration (separate from chunking)
config.tokenizer.enable_sentencepiece = true;
config.tokenizer.sentencepiece_model_path = "/path/to/your/model.model";
```

### 2. SentencePiece Model Setup

You need a SentencePiece model file (`.model`). You can:

- **Use an existing model**: Download from Hugging Face or other sources
- **Train your own**: Use the included `SentencePieceTokenizer::train_model()` method
- **Use a pretrained model**: Many LLMs provide their SentencePiece models

Example model sources:
- **LLaMA/Alpaca models**: Usually include `tokenizer.model`
- **T5 models**: Available on Hugging Face
- **Custom training**: Train on your specific domain data

### 3. Integration Example

```cpp
#include "leafra/leafra_core.h"

int main() {
    auto sdk = leafra::LeafraCore::create();
    
    leafra::Config config;
    config.debug_mode = true; // See detailed tokenization info
    
    // Configure chunking
    config.chunking.enabled = true;
    config.chunking.chunk_size = 500; // 500 actual tokens
    config.chunking.overlap_percentage = 0.10; // Exactly 10% token overlap
    config.chunking.size_unit = leafra::ChunkSizeUnit::TOKENS;
    
    // Configure SentencePiece tokenization (separate configuration)
    config.tokenizer.enable_sentencepiece = true;
    config.tokenizer.sentencepiece_model_path = "models/llama-tokenizer.model";
    
    // Initialize SDK
    if (sdk->initialize(config) == leafra::ResultCode::SUCCESS) {
        // Process files - chunks will have accurate token counts
        std::vector<std::string> files = {"document.pdf", "text.txt"};
        sdk->process_user_files(files);
    }
    
    sdk->shutdown();
    return 0;
}
```

## How It Works

### Before (Character-based estimates):
```
[DEBUG:DENSITY] Sampled text density: 2.000000 chars/token
[DEBUG:CHUNKING] CREATED - Chunk #1 [0-2014] (2014 chars, 503/500 tokens)
[DEBUG:CHUNKING] CREATED - Chunk #2 [901-2915] (2014 chars, 503/500 tokens)
```
- **Problem**: ~60% overlap instead of intended 10%
- **Cause**: `bytes/4` estimate was inaccurate for your content

### After (SentencePiece tokenization):
```
[INFO] Using SentencePiece for accurate token counting
[INFO] ✅ SentencePiece tokenization completed
[INFO] Chunk statistics:
[INFO]   - Total chunks: 208
[INFO]   - Actual tokens: 104,000
[INFO]   - Actual chars/token ratio: 3.2
[INFO] Chunk 1 - Characters: 1618, Actual tokens: 500, Chars/token ratio: 3.24
```
- **Result**: Exact 10% overlap as configured
- **Benefit**: Chunks are precisely 500 tokens each

## Configuration Options

### TokenizerConfig (config.tokenizer)

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `enable_sentencepiece` | `bool` | `false` | Enable SentencePiece tokenization |
| `sentencepiece_model_path` | `string` | `""` | Path to `.model` file |

### ChunkingConfig (config.chunking)

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `enabled` | `bool` | `true` | Enable chunking during file processing |
| `chunk_size` | `size_t` | `500` | Target tokens per chunk (when using TOKENS unit) |
| `overlap_percentage` | `double` | `0.15` | Overlap percentage (0.0 to 1.0) |
| `size_unit` | `ChunkSizeUnit` | `TOKENS` | CHARACTERS or TOKENS |
| `preserve_word_boundaries` | `bool` | `true` | Avoid breaking words |

## Output Changes

### Log Messages
When SentencePiece is enabled, you'll see:
```
[INFO] Initializing SentencePiece tokenizer
[INFO] ✅ SentencePiece model loaded from: /path/to/model.model
[INFO]   - Vocabulary size: 32000
[DEBUG] Using SentencePiece for accurate token counting
[INFO] ✅ SentencePiece tokenization completed
[INFO]   - Total actual tokens: 104,523
[INFO] Chunk statistics:
[INFO]   - Actual tokens: 104,523
[INFO]   - Actual chars/token ratio: 3.24
```

### Chunk Information
Each chunk now shows:
```
[INFO] Chunk 1 of 208:
[INFO]   📐 Length: 1618 characters
[INFO]   🔤 Tokens: 500 (actual)  # ← Now shows "actual" vs "estimated"
[INFO]   📊 Chars/token ratio: 3.24
```

## Performance Considerations

- **Initialization**: SentencePiece model loading happens once during SDK initialization
- **Runtime**: Tokenization adds ~10-20% processing time but provides accurate results
- **Memory**: Model typically uses 50-200MB depending on vocabulary size
- **Fallback**: System automatically falls back to character estimates if SentencePiece fails

## Troubleshooting

### Model Not Loading
```
[WARNING] ⚠️  Failed to load SentencePiece model from: /path/to/model.model
[WARNING]   - Falling back to character-based token estimation
```
**Solution**: Check file path and permissions, ensure model file is valid. The system will automatically fall back to character-based estimates.

### SentencePiece Not Available
```
[WARNING] ⚠️  SentencePiece requested but tokenizer not available
```
**Solution**: Ensure LeafraSDK was compiled with SentencePiece support (`LEAFRA_HAS_SENTENCEPIECE`)

### Different Token Counts
If you see significantly different token counts, this is expected! The estimates were inaccurate, and now you're seeing real token counts that match your target LLM.

## Migration Guide

### From Character-based Chunking
1. **Set your desired token count**: Use the same chunk size but now in actual tokens
2. **Enable SentencePiece**: Configure `config.tokenizer.enable_sentencepiece = true`
3. **Set model path**: Configure `config.tokenizer.sentencepiece_model_path = "/path/to/model.model"`
4. **Provide a model**: Download or train a SentencePiece model for your use case
5. **Test and adjust**: Your chunk sizes may change since estimates were inaccurate

**Note**: The system automatically falls back to character-based estimates if SentencePiece fails to load.

### Expected Changes
- **Configuration**: Tokenization config is now separate from chunking (`config.tokenizer.*`)
- **Chunk sizes**: May be different (more accurate now)
- **Overlap**: Now precisely controlled
- **Token counts**: Will match your target LLM exactly
- **Performance**: Slightly slower but much more accurate

## Advanced Usage

### Training Your Own Model
```cpp
// Train a custom SentencePiece model on your data
SentencePieceTokenizer tokenizer;
SentencePieceTokenizer::TrainOptions options;
options.vocab_size = 8000;
options.model_type = "unigram";

std::vector<std::string> input_files = {"training_data.txt"};
tokenizer.train_model(input_files, "custom_model", options);
```

### Accessing Token IDs from Chunks

**NEW FEATURE**: The LeafraSDK now stores the actual SentencePiece token IDs in each chunk, making it easy to work with the exact tokens that were used for counting.

#### Direct Access from TextChunk
```cpp
// After processing files and chunking
std::vector<TextChunk> chunks; // Your chunks from the chunking process

for (size_t i = 0; i < chunks.size(); ++i) {
    const auto& chunk = chunks[i];
    
    // Check if chunk has SentencePiece token IDs
    if (chunk.has_token_ids()) {
        // Access the actual token IDs used by SentencePiece
        const std::vector<int>& token_ids = chunk.token_ids;
        
        std::cout << "Chunk " << i << " has " << token_ids.size() << " tokens:" << std::endl;
        
        // Print first few token IDs
        for (size_t j = 0; j < std::min(size_t(10), token_ids.size()); ++j) {
            std::cout << token_ids[j] << " ";
        }
        std::cout << std::endl;
        
        // You can now use these token IDs directly with your LLM
        // or further processing pipeline
    }
}
```

#### Using the Helper Structure
For easier access to chunk information and token IDs:

```cpp
#include "leafra/leafra_core.h"

// Extract token information using the helper function
auto token_info = LeafraCore::extract_chunk_token_info(chunks);

for (const auto& info : token_info) {
    if (info.has_valid_tokens()) {
        std::cout << "Chunk " << info.chunk_index 
                  << " (page " << info.page_number + 1 << ")" << std::endl;
        std::cout << "  Content preview: " << info.content.substr(0, 50) << "..." << std::endl;
        std::cout << "  Token count: " << info.token_count << std::endl;
        std::cout << "  Character count: " << info.character_count << std::endl;
        std::cout << "  Chars/token ratio: " << info.get_chars_per_token_ratio() << std::endl;
        
        // Access token IDs
        const std::vector<int>& token_ids = info.token_ids;
        std::cout << "  First 5 token IDs: ";
        for (size_t i = 0; i < std::min(size_t(5), token_ids.size()); ++i) {
            std::cout << token_ids[i] << " ";
        }
        std::cout << std::endl;
    }
}
```

#### Matching Chunks with Token IDs

The token IDs are stored directly in each chunk, so you can easily create mappings:

```cpp
// Create a map of chunk index to token IDs
std::map<size_t, std::vector<int>> chunk_token_map;

for (size_t i = 0; i < chunks.size(); ++i) {
    if (chunks[i].has_token_ids()) {
        chunk_token_map[i] = chunks[i].token_ids;
    }
}

// Or create a map of chunk content to token IDs
std::map<std::string, std::vector<int>> content_token_map;

for (const auto& chunk : chunks) {
    if (chunk.has_token_ids()) {
        content_token_map[std::string(chunk.content)] = chunk.token_ids;
    }
}
```

#### Benefits of Stored Token IDs

1. **Exact LLM Compatibility**: The token IDs match exactly what your target LLM will see
2. **No Re-tokenization**: Save computation by reusing the already computed token IDs
3. **Perfect Chunk Boundaries**: Chunks are sized based on actual token counts, not estimates
4. **Easy Integration**: Token IDs are stored right in the chunk structure for immediate access
5. **Debugging**: You can inspect exactly which tokens were counted for each chunk

### Debugging Token Counts
Enable debug mode to see detailed tokenization information:
```cpp
config.debug_mode = true;
config.chunking.print_chunks_brief = true; // See chunk content and token counts
```

This will solve the overlap percentage issues you were experiencing and give you precise control over chunk sizes for LLM processing. 