#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>

// Platform-specific macros
#ifdef _WIN32
    #ifdef LEAFRA_EXPORTS
        #define LEAFRA_API __declspec(dllexport)
    #elif defined(LEAFRA_SHARED)
        #define LEAFRA_API __declspec(dllimport)
    #else
        #define LEAFRA_API
    #endif
#else
    #ifdef LEAFRA_EXPORTS
        #define LEAFRA_API __attribute__((visibility("default")))
    #else
        #define LEAFRA_API
    #endif
#endif

// Version macros
#define LEAFRA_VERSION_MAJOR 1
#define LEAFRA_VERSION_MINOR 0
#define LEAFRA_VERSION_PATCH 0

namespace leafra {

// Forward declarations
class DataProcessor;
class MathUtils;

// Forward declaration for chunking types
enum class ChunkSizeUnit : int32_t;
enum class TokenApproximationMethod : int32_t;

// Basic types
using byte_t = uint8_t;
using data_buffer_t = std::vector<byte_t>;
using callback_t = std::function<void(const std::string&)>;

// LLM-related callback types
using token_callback_t = std::function<bool(const std::string& token, bool is_final)>;

/**
 * @brief Helper structure for accessing chunk token information
 * 
 * This structure provides easy access to chunk content and its associated 
 * SentencePiece token IDs, making it simple to match chunks with their tokens.
 */
struct LEAFRA_API ChunkTokenInfo {
    size_t chunk_index;                    // Index of the chunk in the original chunks vector
    std::string_view content;              // Reference to the chunk content
    std::vector<int> token_ids;            // SentencePiece token IDs for this chunk
    size_t character_count;                // Number of characters in the chunk
    size_t token_count;                    // Number of tokens (same as token_ids.size())
    size_t page_number;                    // Page number this chunk originated from
    
    ChunkTokenInfo() = default;
    ChunkTokenInfo(size_t idx, std::string_view text, const std::vector<int>& ids, 
                   size_t chars, size_t tokens, size_t page)
        : chunk_index(idx), content(text), token_ids(ids), 
          character_count(chars), token_count(tokens), page_number(page) {}
    
    // Helper method to check if this chunk has valid token IDs
    bool has_valid_tokens() const {
        return !token_ids.empty() && token_count == token_ids.size();
    }
    
    // Helper method to get chars per token ratio
    double get_chars_per_token_ratio() const {
        return token_count > 0 ? static_cast<double>(character_count) / token_count : 0.0;
    }
};

// Result types
enum class ResultCode : int32_t {
    SUCCESS = 0,
    ERROR_INVALID_PARAMETER = -1,
    ERROR_INITIALIZATION_FAILED = -2,
    ERROR_PROCESSING_FAILED = -3,
    ERROR_NOT_IMPLEMENTED = -4,
    ERROR_OUT_OF_MEMORY = -5,
    ERROR_NOT_FOUND = -6
};

/**
 * @brief Tokenizer configuration for the SDK
 */
struct LEAFRA_API TokenizerConfig {
    // SentencePiece tokenizer configuration
    bool enable_sentencepiece = false;      // Whether to use SentencePiece for accurate token counting
    std::string model_name ;         // Model name (corresponds to folder in sdk/corecpp/third_party/models/)
    std::string sentencepiece_model_path;       // Path to SentencePiece model file (.model) - can be set manually or resolved from model_name
    std::string sentencepiece_json_path;        // Path to tokenizer config JSON file (tokenizer_config.json) - resolved from model_name
    // Future tokenizer options can be added here
    // bool enable_tiktoken = false;         // For OpenAI models
    // bool enable_huggingface_tokenizer = false; // For HuggingFace models
    // std::string tokenizer_type = "sentencepiece"; // Default tokenizer type
    
    // Default constructor
    TokenizerConfig() = default;
    
    // Constructor with model name
    TokenizerConfig(const std::string& model_name_param, bool enable = true) 
        : enable_sentencepiece(enable), model_name(model_name_param) {}
    
};

/**
 * @brief Chunking configuration for the SDK
 */
struct LEAFRA_API ChunkingConfig {
    bool enabled = false;                    // Whether to enable chunking during file processing
    size_t chunk_size = 500;               // Size of each chunk (UTF-8 characters or tokens, depending on size_unit)
    double overlap_percentage = 0.15;       // Overlap percentage (0.0 to 1.0)
    bool preserve_word_boundaries = true;   // Whether to avoid breaking words
    bool include_metadata = true;           // Whether to include chunk metadata
    ChunkSizeUnit size_unit;                // Unit for chunk_size (CHARACTERS = UTF-8 chars, TOKENS = approximate)
    TokenApproximationMethod token_method;  // Token approximation method
    
    // Debug/Development options for chunk content printing
    bool print_chunks_full = false;         // Print full content of all chunks
    bool print_chunks_brief = false;        // Print first N lines of each chunk
    int max_lines = 3;                      // Maximum lines to show when print_chunks_brief is true
    
    // Default constructor with sensible defaults for LLM processing
    ChunkingConfig();
    
    // Constructor with custom parameters
    ChunkingConfig(size_t size, double overlap, bool use_tokens = true);
};

/**
 * @brief Embedding model inference configuration
 */
struct LEAFRA_API EmbeddingModelConfig {
    bool enabled = false;                   // Whether to enable embedding model inference
    std::string framework = "";             // Inference framework ("coreml", "tensorflow_lite", or "tensorflow")
    std::string model_path = "";            // Path to the model file (.mlmodel/.mlpackage for CoreML, .tflite for TensorFlow Lite)
    
    // CoreML specific settings (only used when framework = "coreml")
    std::string coreml_compute_units = "all";      // CoreML compute units: "all", "cpuOnly", "cpuAndGPU", "cpuAndNeuralEngine"
    
    // TensorFlow Lite delegate configurations (only used when framework = "tflite")
    bool tflite_enable_coreml_delegate = true;     // Enable CoreML delegate (iOS/macOS only)
    bool tflite_enable_metal_delegate = true;      // Enable Metal GPU delegate (iOS/macOS only)
    bool tflite_enable_xnnpack_delegate = true;    // Enable XNNPACK CPU delegate (cross-platform)
    
    // TensorFlow Lite performance settings (only used when framework = "tflite")
    int32_t tflite_num_threads = -1;               // Number of threads (-1 = auto)
    bool tflite_use_nnapi = false;                 // Use Android NNAPI (Android only)
    
    // Default constructor
    EmbeddingModelConfig() = default;
    
    // Check if configuration is valid
    bool is_valid() const {
        return enabled && !framework.empty() && !model_path.empty() &&
               (framework == "coreml" || framework == "tensorflow_lite" || framework == "tensorflow");
    }
};

#ifdef LEAFRA_HAS_FAISS
// Forward declarations for FAISS types
    class FaissIndex;
#endif

/**
 * @brief Vector search configuration for the SDK
 */
struct LEAFRA_API VectorSearchConfig {
    bool enabled = false;                   // Whether to enable vector search functionality
    int32_t dimension = 384;                // Vector dimension (default for many embedding models)
    std::string index_type = "HNSW";        // FAISS index type: "FLAT", "IVF_FLAT", "IVF_PQ", "HNSW", "LSH"
    std::string metric = "COSINE";          // Distance metric: "L2", "INNER_PRODUCT", "COSINE"
    
    // Advanced FAISS configuration
    int32_t nlist = 100;                    // Number of clusters for IVF indexes (auto-calculated if 0)
    int32_t nprobe = 10;                    // Number of clusters to search in IVF indexes
    int32_t m = 8;                          // Number of subquantizers for PQ indexes
    int32_t nbits = 8;                      // Bits per subquantizer for PQ indexes
    int32_t hnsw_m = 16;                    // Number of bi-directional links for HNSW
    int32_t lsh_nbits = 64;                 // Number of hash bits for LSH
    
    // Database storage configuration
    std::string index_definition = "default"; // Definition string for database storage
    bool auto_save = true;                  // Automatically save index to database after building
    bool auto_load = true;                  // Automatically load index from database on initialization
    
    // Default constructor
    VectorSearchConfig() = default;
    
    // Constructor with basic parameters
    VectorSearchConfig(int32_t dim, const std::string& idx_type = "FLAT", const std::string& metric_type = "COSINE")
        : enabled(true), dimension(dim), index_type(idx_type), metric(metric_type) {}
    
    // Check if configuration is valid
    bool is_valid() const {
        return dimension > 0 && 
               (index_type == "FLAT" || index_type == "IVF_FLAT" || index_type == "IVF_PQ" || 
                index_type == "HNSW" || index_type == "LSH") &&
               (metric == "L2" || metric == "INNER_PRODUCT" || metric == "COSINE");
    }
    
    // Note: FAISS enum conversion methods are implemented in leafra_core.cpp
    // when LEAFRA_HAS_FAISS is defined and leafra_faiss.h is included
};

/**
 * @brief General LLM (Large Language Model) configuration for the SDK
 */
struct LEAFRA_API LLMConfig {
    bool enabled = false;                   // Whether to enable LLM functionality
    std::string model_path = "";            // Path to the LLM model file
    std::string framework = "llamacpp";     // LLM framework: "llamacpp", "ollama", etc.
    
    // Context and processing parameters
    int32_t n_ctx = 4096;                  // Maximum context length in tokens
    int32_t n_predict = 128;               // Maximum tokens to generate per request
    int32_t n_batch = 512;                 // Batch size for processing
    int32_t n_ubatch = 512;                // Physical batch size for prompt processing
    int32_t n_threads = -1;                // Number of threads (-1 = auto-detect)
    int32_t n_threads_batch = -1;          // Number of threads for batch processing (-1 = auto)
    
    // Generation parameters
    float temperature = 0.8f;              // Sampling temperature (0.0 = deterministic, higher = more random)
    float top_p = 0.9f;                    // Nucleus sampling (top-p) probability
    int32_t top_k = 40;                    // Top-k sampling (0 = disabled)
    float min_p = 0.05f;                   // Minimum probability for sampling
    float repeat_penalty = 1.1f;           // Penalty for repeating tokens
    int32_t repeat_last_n = 64;            // Number of previous tokens to consider for repetition penalty
    
    // Advanced sampling parameters (usually not changed)
    float tfs_z = 1.0f;                    // Tail free sampling (1.0 = disabled)
    float typical_p = 1.0f;                // Typical sampling (1.0 = disabled)
    
    // Performance and hardware parameters
    int32_t n_gpu_layers = 32;             // TODO AD: How to set this automatically? Number of layers to offload to GPU (-1 = auto, 0 = CPU only)
    bool use_mmap = true;                  // Use memory mapping for model loading (faster startup)
    bool use_mlock = false;                // Lock model in RAM (prevents swapping)
    bool numa = false;                     // Enable NUMA optimization
    
    // System configuration
    std::string system_prompt = "";        // System prompt to use for conversations
    int32_t seed = -1;                     // Random seed for reproducible outputs (-1 = random)
    bool debug_mode = false;               // Enable debug output
    bool verbose_prompt = false;           // Print prompt before generation
    
    // Default constructor
    LLMConfig() = default;
    
    // Constructor with model path
    explicit LLMConfig(const std::string& model_path_param) 
        : enabled(true), model_path(model_path_param) {}
    
    // Check if configuration is valid
    bool is_valid() const {
        return enabled && !model_path.empty() && !framework.empty() &&
               n_ctx > 0 && n_predict > 0 && n_batch > 0 && n_ubatch > 0 &&
               temperature >= 0.0f && top_p > 0.0f && top_p <= 1.0f &&
               repeat_penalty > 0.0f && min_p >= 0.0f && min_p <= 1.0f;
    }
    
    // Helper method to get model filename from path
    std::string get_model_filename() const {
        size_t pos = model_path.find_last_of("/\\");
        return (pos == std::string::npos) ? model_path : model_path.substr(pos + 1);
    }
};

// Configuration structure
struct LEAFRA_API Config {
    std::string name;
    std::string version;
    bool debug_mode = false;
    int32_t max_threads = 4;
    size_t buffer_size = 1024;
    std::string leafra_document_database_name = "leafra.db"; // Document database filename
    ChunkingConfig chunking;               // Chunking configuration
    TokenizerConfig tokenizer;             // Tokenization configuration
    EmbeddingModelConfig embedding_inference; // Embedding model inference configuration
    VectorSearchConfig vector_search;       // Vector search configuration
    LLMConfig llm;                         // Large Language Model configuration
};

// Data structures
struct Point2D {
    double x = 0.0;
    double y = 0.0;
    
    Point2D() = default;
    Point2D(double x_val, double y_val) : x(x_val), y(y_val) {}
};

struct Point3D {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    
    Point3D() = default;
    Point3D(double x_val, double y_val, double z_val) : x(x_val), y(y_val), z(z_val) {}
};

struct Matrix3x3 {
    double data[9] = {0.0};
    
    Matrix3x3() {
        for (int i = 0; i < 9; ++i) {
            data[i] = 0.0;
        }
    }
    
    explicit Matrix3x3(const double* values) {
        for (int i = 0; i < 9; ++i) {
            data[i] = values[i];
        }
    }
    
    double& operator()(int row, int col) {
        return data[row * 3 + col];
    }
    
    const double& operator()(int row, int col) const {
        return data[row * 3 + col];
    }
};

// Event types
enum class EventType : int32_t {
    INITIALIZATION_COMPLETE = 0,
    DATA_PROCESSED = 1,
    ERROR_OCCURRED = 2,
    CUSTOM_EVENT = 100
};

struct Event {
    EventType type;
    std::string message;
    int64_t timestamp;
    data_buffer_t data;
    
    Event(EventType t, const std::string& msg) 
        : type(t), message(msg), timestamp(0) {}
};

// Smart pointer aliases
template<typename T>
using unique_ptr = std::unique_ptr<T>;

template<typename T>
using shared_ptr = std::shared_ptr<T>;

// Utility functions
LEAFRA_API const char* result_code_to_string(ResultCode code);
LEAFRA_API std::string get_version_string();
LEAFRA_API int64_t get_current_timestamp();

} // namespace leafra 