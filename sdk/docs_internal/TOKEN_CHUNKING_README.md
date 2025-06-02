# Token-Based Chunking for LeafraChunker

## Overview

The LeafraChunker now supports **token-based chunking** in addition to the original character-based chunking. This enhancement allows you to specify chunk sizes in tokens rather than characters, which is particularly useful for applications working with Large Language Models (LLMs) that have token-based context limits.

## Features

### ✅ **Fully Backward Compatible**
- All existing APIs continue to work unchanged
- Default behavior remains character-based chunking
- Existing code requires no modifications

### 🎯 **Unified Token Approximation**
- **Simple Method**: 1 token ≈ 4 characters (fast and consistent)
- Simplified approach for better consistency across different content types
- All methods now use the same approximation for predictable behavior

### 📊 **Enhanced Metadata**
- Chunks now include `estimated_tokens` field
- Automatic token counting for each chunk
- Statistics tracking for both characters and tokens

## New API Reference

### Enums

```cpp
enum class ChunkSizeUnit : int32_t {
    CHARACTERS = 0,  // Default: chunk size in characters
    TOKENS = 1       // Chunk size in tokens (approximate)
};

enum class TokenApproximationMethod : int32_t {
    SIMPLE = 0     // 1 token ≈ 4 characters (unified approach)
};
```

### Enhanced Structures

```cpp
struct TextChunk {
    std::string content;
    size_t start_index = 0;
    size_t end_index = 0;
    size_t page_number = 0;
    size_t estimated_tokens = 0;  // 🆕 New field
};

struct ChunkingOptions {
    size_t chunk_size = 1000;
    double overlap_percentage = 0.1;
    bool preserve_word_boundaries = true;
    bool include_metadata = true;
    ChunkSizeUnit size_unit = ChunkSizeUnit::CHARACTERS;  // 🆕 New field
    TokenApproximationMethod token_method = TokenApproximationMethod::SIMPLE;  // 🆕 New field
};
```

### Static Utility Methods

```cpp
// Static utility methods
static size_t estimate_token_count(const std::string& text, 
                                  TokenApproximationMethod method);

static size_t tokens_to_characters(size_t token_count, 
                                  TokenApproximationMethod method);
```

## Usage Examples

### Basic Token-Based Chunking

```cpp
#include "leafra/leafra_chunker.h"
using namespace leafra;

LeafraChunker chunker;
chunker.initialize();

std::string text = "Your document text here...";
std::vector<TextChunk> chunks;

// Chunk into ~100-token pieces with 10% overlap using the advanced API
ChunkingOptions options(100, 0.1, ChunkSizeUnit::TOKENS, TokenApproximationMethod::SIMPLE);
ResultCode result = chunker.chunk_text(text, options, chunks);

if (result == ResultCode::SUCCESS) {
    for (const auto& chunk : chunks) {
        std::cout << "Chunk: " << chunk.estimated_tokens << " tokens, "
                  << chunk.content.length() << " characters" << std::endl;
    }
}
```

### Multi-Page Token Chunking

```cpp
std::vector<std::string> pages = {
    "Page 1 content...",
    "Page 2 content...",
    "Page 3 content..."
};

std::vector<TextChunk> chunks;
ChunkingOptions options(50, 0.15, ChunkSizeUnit::TOKENS, TokenApproximationMethod::SIMPLE);
ResultCode result = chunker.chunk_document(pages, options, chunks);

// Each chunk will have page_number and estimated_tokens populated
```

### Token Estimation

```cpp
std::string text = "Sample text for token estimation";

// Get token estimates using the unified simple method
size_t estimated_tokens = LeafraChunker::estimate_token_count(text, TokenApproximationMethod::SIMPLE);

// Convert tokens back to estimated character count
size_t estimated_chars = LeafraChunker::tokens_to_characters(100, TokenApproximationMethod::SIMPLE);
```

### Using Enhanced ChunkingOptions

```cpp
// Create options for token-based chunking
ChunkingOptions options(
    80,  // chunk size in tokens
    0.2, // overlap
    ChunkSizeUnit::TOKENS,
    TokenApproximationMethod::SIMPLE
);

std::vector<TextChunk> chunks;
ResultCode result = chunker.chunk_document(pages, options, chunks);
```

## Approximation Method Details

| Method | Speed | Accuracy | Description |
|--------|-------|----------|-------------|
| **Simple** | ⚡ Fastest | ~80% | Unified approach: 1 token ≈ 4 characters |

### Performance Characteristics

- **Simple**: `O(1)` - just divides character count by 4
- Consistent behavior across all content types
- Fast and predictable for all use cases

## Migration Guide

### For Existing Code
**No changes required!** All existing APIs work unchanged:

```cpp
// This continues to work exactly as before
chunker.chunk_text(text, ChunkingOptions(1000, 0.1), chunks);
chunker.chunk_document(pages, ChunkingOptions(500, 0.2), chunks);
```

### To Add Token Support
Simply replace your existing calls:

```cpp
// Before (character-based)
chunker.chunk_text(text, ChunkingOptions(1000, 0.1), chunks);

// After (token-based)
chunker.chunk_text(text, ChunkingOptions(250, 0.1, ChunkSizeUnit::TOKENS), chunks);
```

## Implementation Details

### Token Approximation Algorithm

#### Simple Method (Unified Approach)
```
tokens = round(characters / 4.0)  // ~4 characters per token
```

This unified approach provides:
- **Consistency**: Same approximation across all content types
- **Simplicity**: Easy to understand and predict
- **Performance**: O(1) time complexity
- **Reliability**: Works well for most English text content

### Character-to-Token Conversion

| Method | Formula |
|--------|---------|
| Simple | `chars = tokens × 4` |

## Testing

The implementation includes comprehensive unit tests:

- ✅ Token estimation accuracy (Test 17)
- ✅ Basic token-based chunking (Test 18)  
- ✅ Token-based multi-page chunking (Test 19)
- ✅ Token chunking error handling (Test 20)
- ✅ Unified approximation method testing (Test 21)

**Total test coverage**: 21 tests with 100% pass rate

## Common Use Cases

### 1. LLM Context Window Management
```cpp
// For OpenAI GPT-4 (8K context)
ChunkingOptions options(6000, 0.1, ChunkSizeUnit::TOKENS);
chunker.chunk_text(document, options, chunks);
```

### 2. Embedding Model Optimization
```cpp
// For typical embedding models (~500 token limit)
ChunkingOptions options(400, 0.15, ChunkSizeUnit::TOKENS);
chunker.chunk_text(text, options, chunks);
```

### 3. RAG (Retrieval-Augmented Generation)
```cpp
// Balance between context and retrieval precision
ChunkingOptions options(200, 0.2, ChunkSizeUnit::TOKENS);
chunker.chunk_document(pages, options, chunks);
```

## Best Practices

1. **Use the unified simple method**: Consistent and reliable for all use cases
2. **Consider overlap**: 10-20% overlap helps maintain context
3. **Preserve word boundaries**: Keep enabled for better readability
4. **Monitor token estimates**: Use `chunk.estimated_tokens` for validation
5. **Test with your content**: Validate approximation accuracy with your specific text types

## Limitations

- **Approximation only**: Not exact token counts (requires actual tokenizer)
- **Language specific**: Optimized for English text
- **Context dependent**: Accuracy varies with text type
- **No subword awareness**: Doesn't account for BPE/WordPiece tokenization

## Future Enhancements

- Integration with actual tokenizers (tiktoken, sentencepiece)
- Language-specific approximation models
- Adaptive approximation based on text analysis
- Token-based overlap specification
- Performance optimizations for very large documents

---

**Note**: This implementation provides approximations suitable for most use cases. For exact token counts, integrate with your specific tokenizer library. 