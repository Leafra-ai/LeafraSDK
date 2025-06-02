#pragma once

#include "types.h"
#include "../../src/leafra/leafra_unicode.h"
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <memory>

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
    CHARACTERS = 0,  // Chunk size in UTF-8 characters (Unicode code points, not bytes)
    TOKENS = 1       // Chunk size in tokens (approximate)
};

/**
 * @brief Enum for token approximation methods
 * NOTE: Simplified to use single unified approach of ~4 chars/token for consistency.
 */
enum class TokenApproximationMethod : int32_t {
    SIMPLE = 0     // 1 token ≈ 4 characters (unified approach)
};

/**
 * @brief Text chunk with metadata using string_view to avoid copying
 */
struct TextChunk {
    std::string_view content;       // View into original text - no copying!
    size_t start_index = 0;
    size_t end_index = 0;
    size_t page_number = 0;
    size_t estimated_tokens = 0;  // Estimated token count for this chunk
    
    // SentencePiece token IDs (empty if SentencePiece not used)
    std::vector<int> token_ids;   // Actual token IDs from SentencePiece encoding
    
    TextChunk() = default;
    TextChunk(std::string_view text, size_t start, size_t end, size_t page = 0)
        : content(text), start_index(start), end_index(end), page_number(page), estimated_tokens(0) {}
        
    // Helper method to get content as string when needed
    std::string to_string() const {
        return std::string(content);
    }
    
    // Helper method to check if chunk has actual token IDs
    bool has_token_ids() const {
        return !token_ids.empty();
    }
};

/**
 * @brief Chunking options structure
 */
struct ChunkingOptions {
    size_t chunk_size = 500;          // Size of each chunk (UTF-8 characters or tokens depending on size_unit)
    double overlap_percentage = 0.1;    // Overlap percentage (0.0 to 1.0)
    bool preserve_word_boundaries = true; // Whether to avoid breaking words
    bool include_metadata = true;       // Whether to include chunk metadata
    ChunkSizeUnit size_unit = ChunkSizeUnit::TOKENS;  // Unit for chunk_size (CHARACTERS = UTF-8 chars, TOKENS = approximate)
    TokenApproximationMethod token_method = TokenApproximationMethod::SIMPLE;  // Token approximation method
    
    ChunkingOptions() = default;
    ChunkingOptions(size_t size, double overlap)
        : chunk_size(size), overlap_percentage(overlap) {}
    
    // New constructor for token-based chunking
    ChunkingOptions(size_t size, double overlap, ChunkSizeUnit unit, TokenApproximationMethod method = TokenApproximationMethod::SIMPLE)
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
     * @brief Advanced chunking with options
     * @param text Input text to chunk
     * @param options Chunking options
     * @param chunks Output vector of text chunks
     * @return ResultCode indicating success or failure
     */
    ResultCode chunk_text(const std::string& text,
                         const ChunkingOptions& options,
                         std::vector<TextChunk>& chunks);

    
    // ========== DOCUMENT CHUNKING METHODS ==========
    
    /**
     * @brief Advanced chunking with options
     * @param pages Vector of text pages
     * @param options Chunking options
     * @param chunks Output vector of text chunks
     * @return ResultCode indicating success or failure
     */
    ResultCode chunk_document(const std::vector<std::string>& pages,
                             const ChunkingOptions& options,
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
     * @brief Estimate token count from text
     * @param text Text to analyze (as string_view for efficiency)
     * @param method Token approximation method
     * @return Estimated token count
     */
    static size_t estimate_token_count(std::string_view text, TokenApproximationMethod method);
    
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
                          size_t global_start);
    
    /**
     * @brief Advanced token approximation with heuristics
     */
    static size_t estimate_tokens_advanced(const std::string& text);

    // ========== IMPROVED TOKEN CHUNKING METHODS (PRIVATE) ==========
    
    /**
     * @brief Improved token-based chunking for single text
     * @param text Input text to chunk
     * @param options Chunking options
     * @param chunks Output vector of text chunks
     * @return ResultCode indicating success or failure
     */
    ResultCode actual_chunker(const std::string& text,
                             const ChunkingOptions& options,
                             std::vector<TextChunk>& chunks);
    
    /**
     * @brief Find optimal chunk end position based on target token count
     * @param text Source text
     * @param start_pos Starting position
     * @param target_tokens Target number of tokens
     * @param options Chunking options
     * @param text_unicode_length Pre-calculated Unicode length of the text
     * @param chars_per_token Sampled characters per token ratio for this text
     * @return Optimal end position
     */
    size_t find_optimal_chunk_end(const std::string& text,
                                 size_t start_pos,
                                 size_t target_tokens,
                                 const ChunkingOptions& options,
                                 size_t text_unicode_length,
                                 double chars_per_token) const;
    
    /**
     * @brief Estimate character count needed for target tokens
     * @param text Source text for context
     * @param start_pos Starting position
     * @param target_tokens Target number of tokens
     * @param method Token approximation method
     * @param text_unicode_length Pre-calculated Unicode length of the text
     * @return Estimated character count
     */
    size_t estimate_characters_for_tokens(const std::string& text,
                                         size_t start_pos,
                                         size_t target_tokens,
                                         TokenApproximationMethod method,
                                         size_t text_unicode_length) const;
    
    /**
     * @brief Find the start of the next word from given position
     * @param text Source text
     * @param pos Current position
     * @return Position of next word start
     */
    size_t find_next_word_start(const std::string& text, size_t pos) const;
    
    /**
     * @brief Sample text density to learn actual characters per token ratio
     * @param text The text to sample from
     * @param options Chunking options
     * @return Actual characters per token ratio for this text
     */
    double sample_text_density(const std::string& text, const ChunkingOptions& options) const;

private:
    ChunkingOptions default_options_;
    size_t last_chunk_count_ = 0;
    size_t last_total_characters_ = 0;
    
    // Unicode caching for performance optimization
    UnicodeCacher documentCacher;
    UnicodeCacher chunkCacher;
};

} // namespace leafra 