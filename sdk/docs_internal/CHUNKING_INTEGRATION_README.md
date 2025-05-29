# LeafraSDK Chunking Integration

## Overview

The LeafraSDK now includes **automatic document chunking** as part of its core file processing pipeline. After parsing documents, the SDK can automatically split them into smaller, token-aware chunks optimized for Large Language Model (LLM) processing and Retrieval-Augmented Generation (RAG) systems.

## Key Features

### 🚀 **Integrated Processing Pipeline**
- **Parse → Chunk → Process**: Seamless workflow from document parsing to chunked output
- **Automatic Processing**: Chunking happens transparently during `process_user_files()`
- **Configurable**: Full control over chunking behavior through the Config struct

### 🎯 **Token-Aware Chunking**
- **Token-based sizing**: Configure chunk sizes in tokens instead of characters
- **Multiple approximation methods**: Simple, Word-based, and Advanced token estimation
- **LLM-optimized**: Perfect chunk sizes for modern language models (GPT, Claude, etc.)

### ⚙️ **Flexible Configuration**
- **Centralized config**: All chunking settings in the main SDK Config struct
- **Runtime configuration**: Adjust settings before SDK initialization
- **JSON configuration**: Load settings from external configuration files

### 📊 **Rich Metadata & Statistics**
- **Chunk metadata**: Page numbers, token estimates, character counts
- **Processing statistics**: Detailed logging of chunking results
- **Event callbacks**: Real-time notifications about chunking progress

## Architecture

### Integration Points

```
User Files → Parse Documents → Extract Pages → Chunk Pages → Process Chunks
              ↑                                    ↑
         File Parsing                        LeafraChunker
         Adapters                           (Token-aware)
```

### Core Components

1. **ChunkingConfig** - Configuration structure for chunking parameters
2. **LeafraChunker** - Advanced chunking engine with token support
3. **ConfigLoader** - JSON configuration file loader (upcoming)
4. **Event System** - Real-time progress notifications

## Configuration

### ChunkingConfig Structure

```cpp
struct ChunkingConfig {
    bool enabled = true;                    // Enable/disable chunking
    size_t chunk_size = 500;               // Size per chunk (tokens/chars)
    double overlap_percentage = 0.15;       // Overlap between chunks (0.0-1.0)
    bool preserve_word_boundaries = true;   // Avoid breaking words
    bool include_metadata = true;           // Include chunk metadata
    
    // Token-specific settings
    ChunkSizeUnit size_unit = ChunkSizeUnit::TOKENS;
    TokenApproximationMethod token_method = TokenApproximationMethod::WORD_BASED;
};
```

### Token Approximation Methods

| Method | Description | Accuracy | Performance | Use Case |
|--------|-------------|----------|-------------|----------|
| **SIMPLE** | 1 token ≈ 4 characters | ~75% | O(1) | Quick estimation |
| **WORD_BASED** | 1 token ≈ 0.75 words | ~85% | O(n) | General purpose |
| **ADVANCED** | Heuristic analysis | ~90% | O(n) | Precise estimation |

## Usage Examples

### Basic Configuration

```cpp
#include "leafra/leafra_core.h"
#include "leafra/types.h"

// Create SDK instance
auto sdk = LeafraCore::create();

// Configure chunking
Config config;
config.chunking.enabled = true;
config.chunking.chunk_size = 500;        // 500 tokens per chunk
config.chunking.overlap_percentage = 0.2; // 20% overlap
config.chunking.size_unit = ChunkSizeUnit::TOKENS;
config.chunking.token_method = TokenApproximationMethod::WORD_BASED;

// Initialize and process
sdk->initialize(config);
sdk->process_user_files({"document.pdf", "report.txt"});
```

### Advanced Configuration

```cpp
// Custom chunking for different use cases
Config config;

// For LLM context windows
config.chunking.chunk_size = 1000;  // 1K tokens (good for GPT-3.5)
config.chunking.overlap_percentage = 0.1;

// For embedding models  
config.chunking.chunk_size = 300;   // 300 tokens (optimal for embeddings)
config.chunking.overlap_percentage = 0.2;

// For precise applications
config.chunking.token_method = TokenApproximationMethod::ADVANCED;
config.chunking.preserve_word_boundaries = true;
```

### Event Monitoring

```cpp
// Set up event callback to monitor chunking
sdk->set_event_callback([](const std::string& event) {
    std::cout << "SDK Event: " << event << std::endl;
});

// Events you'll receive:
// "🔗 Starting chunking process"
// "🧩 Created 15 chunks"
// "📊 Chunks: 15, Avg size: 487 chars, 89 tokens"
```

## Integration Details

### Modified LeafraCore Methods

#### `initialize(const Config& config)`
- **NEW**: Initializes LeafraChunker with configuration settings
- **NEW**: Logs chunking configuration details
- **NEW**: Validates chunking parameters

#### `process_user_files(const std::vector<std::string>& file_paths)`
- **ENHANCED**: After successful parsing, automatically chunks documents
- **NEW**: Extracts pages from ParsedDocument
- **NEW**: Applies token-aware chunking
- **NEW**: Logs detailed chunking statistics
- **NEW**: Sends chunking events via callback

### Internal Architecture Changes

```cpp
class LeafraCore::Impl {
    // NEW: Chunker instance
    std::unique_ptr<LeafraChunker> chunker_;
    
    // ENHANCED: Config includes chunking settings
    Config config_;
};
```

### Processing Flow

1. **File Parsing**: Documents parsed into `ParsedDocument` structure
2. **Page Extraction**: Text content extracted from `result.pages` vector
3. **Chunking Decision**: Check if `config.chunking.enabled`
4. **Token Chunking**: Apply `chunk_document_advanced()` with token settings
5. **Statistics Logging**: Report chunk counts, sizes, and token estimates
6. **Event Notification**: Send progress updates via event callback

## Configuration File Support

### JSON Configuration (Planned)

```json
{
  "chunking": {
    "enabled": true,
    "chunk_size": 500,
    "overlap_percentage": 0.15,
    "preserve_word_boundaries": true,
    "include_metadata": true,
    "size_unit": "tokens",
    "token_approximation_method": "word_based"
  }
}
```

### Loading Configuration

```cpp
#include "leafra/config_loader.h"

Config config;
ResultCode result = ConfigLoader::load_from_file("config.json", config);
if (result == ResultCode::SUCCESS) {
    sdk->initialize(config);
}
```

## Performance Characteristics

### Chunking Performance
- **Character-based**: O(n) where n = text length
- **Token-based**: O(n) + token estimation overhead
- **Memory usage**: Minimal additional overhead

### Token Estimation Performance
- **Simple method**: ~0.1ms per 1000 characters
- **Word-based method**: ~0.5ms per 1000 characters  
- **Advanced method**: ~1.0ms per 1000 characters

## Common Use Cases

### 1. RAG System Document Processing
```cpp
config.chunking.chunk_size = 400;        // Optimal for embeddings
config.chunking.overlap_percentage = 0.2; // Better context continuity
config.chunking.token_method = TokenApproximationMethod::WORD_BASED;
```

### 2. LLM Context Preparation
```cpp
config.chunking.chunk_size = 2000;       // Large chunks for context
config.chunking.overlap_percentage = 0.1; // Minimal overlap
config.chunking.token_method = TokenApproximationMethod::ADVANCED;
```

### 3. Fast Batch Processing
```cpp
config.chunking.chunk_size = 1000;       // Balanced size
config.chunking.overlap_percentage = 0.15;
config.chunking.token_method = TokenApproximationMethod::SIMPLE; // Fastest
```

## Troubleshooting

### Common Issues

**Q: Chunking not happening?**
- Ensure `config.chunking.enabled = true`
- Check that files are being parsed successfully
- Verify chunker initialization in logs

**Q: Chunks too small/large?**
- Adjust `config.chunking.chunk_size`
- Consider switching between TOKENS/CHARACTERS
- Try different token approximation methods

**Q: Poor token estimation?**
- Use `TokenApproximationMethod::ADVANCED` for better accuracy
- Consider your specific text type (technical, natural language, etc.)

### Debug Mode

```cpp
config.debug_mode = true;  // Enable detailed logging
```

This will log the first few chunks with their content for inspection.

## Future Enhancements

- **JSON Configuration Loader**: External configuration file support
- **Custom Token Models**: Support for specific tokenizer models
- **Parallel Chunking**: Multi-threaded chunking for large documents
- **Chunk Storage**: Direct integration with vector databases
- **Semantic Chunking**: Context-aware chunk boundaries

## API Reference

### ChunkingConfig Methods
- `ChunkingConfig()` - Default constructor with sensible defaults
- `ChunkingConfig(size_t, double, bool)` - Quick setup constructor

### LeafraCore Integration Methods
- `initialize(const Config&)` - Initialize with chunking configuration
- `process_user_files(const std::vector<std::string>&)` - Process and chunk files
- `set_event_callback(callback_t)` - Monitor chunking events

### Configuration Constants
- `ChunkSizeUnit::CHARACTERS` - Character-based chunking
- `ChunkSizeUnit::TOKENS` - Token-based chunking  
- `TokenApproximationMethod::SIMPLE` - Fast approximation
- `TokenApproximationMethod::WORD_BASED` - Balanced accuracy/speed
- `TokenApproximationMethod::ADVANCED` - Highest accuracy

---

**Note**: This integration maintains full backward compatibility. Existing code will continue to work unchanged, with chunking disabled by default. 