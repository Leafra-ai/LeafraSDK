#pragma once

#include "types.h"
#include <string>
#include <vector>
#include <cstdint>

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

namespace leafra {

/**
 * @brief Forward declarations
 */
class LeafraChunker;

/**
 * @brief Result code enum
 * Note: This enum is defined in types.h
 */

/**
 * @brief Enum for chunk size units
 */
enum class ChunkSizeUnit : int32_t {
    CHARACTERS = 0,  // Default: chunk size in characters
    TOKENS = 1       // Chunk size in tokens (approximate)
};

/**
 * @brief Enum for token approximation methods
 */
enum class TokenApproximationMethod : int32_t {
    SIMPLE = 0,     // 1 token ≈ 4 characters (fastest)
    WORD_BASED = 1, // 1 token ≈ 0.75 words (balanced)
    ADVANCED = 2    // Heuristic based on word length (most accurate)
};

/**
 * @brief Text chunk with metadata
 */
struct TextChunk {
    std::string content;
    size_t start_index = 0;
    size_t end_index = 0;
    size_t page_number = 0;
    size_t estimated_tokens = 0;  // Estimated token count for this chunk
    
    TextChunk() = default;
    TextChunk(const std::string& text, size_t start, size_t end, size_t page = 0)
        : content(text), start_index(start), end_index(end), page_number(page), estimated_tokens(0) {}
};

/**
 * @brief Chunking options structure
 */
struct ChunkingOptions {
    size_t chunk_size = 1000;          // Size of each chunk (characters or tokens)
    double overlap_percentage = 0.1;    // Overlap percentage (0.0 to 1.0)
    bool preserve_word_boundaries = true; // Whether to avoid breaking words
    bool include_metadata = true;       // Whether to include chunk metadata
    ChunkSizeUnit size_unit = ChunkSizeUnit::CHARACTERS;  // Unit for chunk_size
    TokenApproximationMethod token_method = TokenApproximationMethod::WORD_BASED;  // Token approximation method
    
    ChunkingOptions() = default;
    ChunkingOptions(size_t size, double overlap)
        : chunk_size(size), overlap_percentage(overlap) {}
    
    // New constructor for token-based chunking
    ChunkingOptions(size_t size, double overlap, ChunkSizeUnit unit, TokenApproximationMethod method = TokenApproximationMethod::WORD_BASED)
        : chunk_size(size), overlap_percentage(overlap), size_unit(unit), token_method(method) {}
};

/**
 * @brief Text chunking utility class
 * 
 * This class provides functionality to split text documents into chunks
 * with configurable size and overlap. It can handle single documents
 * or multi-page documents represented as vectors of text.
 */
class LEAFRA_API LeafraChunker {
public:
    LeafraChunker();
    ~LeafraChunker();
    
    /**
     * @brief Initialize the chunker
     * @return ResultCode indicating success or failure
     */
    ResultCode initialize();
    
    // ========== TEXT CHUNKING METHODS ==========
    
    /**
     * @brief Chunk a single text document
     * @param text Input text to chunk
     * @param chunk_size Size of each chunk in characters
     * @param overlap_percentage Overlap percentage (0.0 to 1.0)
     * @param chunks Output vector of text chunks
     * @return ResultCode indicating success or failure
     */
    ResultCode chunk_text(const std::string& text,
                         size_t chunk_size,
                         double overlap_percentage,
                         std::vector<TextChunk>& chunks);

    /**
     * @brief Advanced chunking with options (single text)
     * @param text Input text to chunk
     * @param options Chunking options (supports both character and token chunking)
     * @param chunks Output vector of text chunks
     * @return ResultCode indicating success or failure
     */
    ResultCode chunk_text_advanced(const std::string& text,
                                  const ChunkingOptions& options,
                                  std::vector<TextChunk>& chunks);

    /**
     * @brief Chunk a single text document (token-based) [LEGACY - use chunk_text_advanced instead]
     * @param text Input text to chunk
     * @param chunk_size_tokens Size of each chunk in tokens
     * @param overlap_percentage Overlap percentage (0.0 to 1.0)
     * @param method Token approximation method
     * @param chunks Output vector of text chunks
     * @return ResultCode indicating success or failure
     */
    ResultCode chunk_text_tokens(const std::string& text,
                                size_t chunk_size_tokens,
                                double overlap_percentage,
                                TokenApproximationMethod method,
                                std::vector<TextChunk>& chunks);
    
    // ========== DOCUMENT CHUNKING METHODS ==========
    
    /**
     * @brief Chunk a multi-page document
     * @param pages Vector of text pages
     * @param chunk_size Size of each chunk in characters
     * @param overlap_percentage Overlap percentage (0.0 to 1.0)
     * @param chunks Output vector of text chunks
     * @return ResultCode indicating success or failure
     */
    ResultCode chunk_document(const std::vector<std::string>& pages,
                             size_t chunk_size,
                             double overlap_percentage,
                             std::vector<TextChunk>& chunks);

    /**
     * @brief Advanced chunking with options
     * @param pages Vector of text pages
     * @param options Chunking options
     * @param chunks Output vector of text chunks
     * @return ResultCode indicating success or failure
     */
    ResultCode chunk_document_advanced(const std::vector<std::string>& pages,
                                      const ChunkingOptions& options,
                                      std::vector<TextChunk>& chunks);
    
    /**
     * @brief Chunk a multi-page document (token-based) [LEGACY - use chunk_document_advanced instead]
     * @param pages Vector of text pages
     * @param chunk_size_tokens Size of each chunk in tokens
     * @param overlap_percentage Overlap percentage (0.0 to 1.0)
     * @param method Token approximation method
     * @param chunks Output vector of text chunks
     * @return ResultCode indicating success or failure
     */
    ResultCode chunk_document_tokens(const std::vector<std::string>& pages,
                                    size_t chunk_size_tokens,
                                    double overlap_percentage,
                                    TokenApproximationMethod method,
                                    std::vector<TextChunk>& chunks);

    // ========== STATISTICS AND CONFIGURATION ==========
    
    /**
     * @brief Get statistics about the last chunking operation
     * @return Number of chunks created
     */
    size_t get_chunk_count() const;
    
    /**
     * @brief Get total characters processed in last operation
     * @return Total character count
     */
    size_t get_total_characters() const;
    
    /**
     * @brief Reset chunking statistics
     */
    void reset_statistics();
    
    /**
     * @brief Set default chunking options
     * @param options Default options to use
     */
    void set_default_options(const ChunkingOptions& options);
    
    /**
     * @brief Get current default options
     * @return Current default chunking options
     */
    const ChunkingOptions& get_default_options() const;

    // ========== TOKEN UTILITIES ==========

    /**
     * @brief Estimate token count for a given text
     * @param text Input text
     * @param method Approximation method to use
     * @return Estimated token count
     */
    static size_t estimate_token_count(const std::string& text, TokenApproximationMethod method);
    
    /**
     * @brief Convert tokens to approximate character count
     * @param token_count Number of tokens
     * @param method Approximation method used
     * @return Approximate character count
     */
    static size_t tokens_to_characters(size_t token_count, TokenApproximationMethod method);

private:
    /**
     * @brief Find the best split position respecting word boundaries
     * @param text Text to search in
     * @param target_position Target position to split at
     * @param search_window Window size to search for word boundary
     * @return Best position to split at
     */
    size_t find_word_boundary(const std::string& text, 
                             size_t target_position, 
                             size_t search_window = 50) const;
    
    /**
     * @brief Create a chunk from text with metadata
     * @param text Source text
     * @param start Start position in text
     * @param end End position in text
     * @param page_number Page number (0-based)
     * @param global_start Global start position across all pages
     * @return TextChunk object
     */
    TextChunk create_chunk(const std::string& text,
                          size_t start,
                          size_t end,
                          size_t page_number,
                          size_t global_start) const;
    
    /**
     * @brief Simple token approximation: 1 token ≈ 4 characters
     */
    static size_t estimate_tokens_simple(const std::string& text);
    
    /**
     * @brief Word-based token approximation: 1 token ≈ 0.75 words
     */
    static size_t estimate_tokens_word_based(const std::string& text);
    
    /**
     * @brief Advanced token approximation with heuristics
     */
    static size_t estimate_tokens_advanced(const std::string& text);

private:
    ChunkingOptions default_options_;
    size_t last_chunk_count_ = 0;
    size_t last_total_characters_ = 0;
};

} // namespace leafra 