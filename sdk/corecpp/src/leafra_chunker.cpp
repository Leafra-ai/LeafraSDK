#include "leafra/leafra_chunker.h"
#include "leafra/leafra_unicode.h"
#include "leafra/leafra_debug.h"
#include <algorithm>
#include <cctype>
#include <cmath>

namespace leafra {

// Token-to-character conversion constant
// Based on empirical analysis: ~4 characters per token works well across different content types
constexpr double SIMPLE_CHARS_PER_TOKEN = 4.0;

/**
 * Ensure a byte position is aligned to a valid UTF-8 character boundary
 * @param text The UTF-8 text
 * @param pos The byte position to check/adjust
 * @return A valid UTF-8 character start position
 */
static size_t ensure_utf8_boundary(const std::string& text, size_t pos) {
    if (pos >= text.length()) {
        return text.length();
    }
    
    // If we're at a UTF-8 continuation byte, move forward to the next character start
    while (pos < text.length() && (text[pos] & 0xC0) == 0x80) {
        pos++; // Skip UTF-8 continuation bytes
    }
    
    return pos;
}

LeafraChunker::LeafraChunker() = default;

LeafraChunker::~LeafraChunker() = default;

ResultCode LeafraChunker::initialize() {
    // Initialize default options
    default_options_ = ChunkingOptions();
    reset_statistics();
    return ResultCode::SUCCESS;
}

// TEXT CHUNKING WRAPPER API
ResultCode LeafraChunker::chunk_text(const std::string& text,
                                    const ChunkingOptions& options,
                                    std::vector<TextChunk>& chunks) {
    LEAFRA_DEBUG_LOG("CHUNKING", "Starting text chunking with " + std::to_string(text.length()) + " characters");
    
    // Simple wrapper: Convert single text to single-page document
    std::vector<std::string> pages;
    pages.push_back(text);
    ResultCode result = chunk_document(pages, options, chunks);
    
    return result;
}




// DOCUMENT CHUNKING API

ResultCode LeafraChunker::chunk_document(const std::vector<std::string>& pages,
                                                 const ChunkingOptions& options,
                                                 std::vector<TextChunk>& chunks) {
    LEAFRA_DEBUG_TIMER("chunk_document");
    
    if (pages.empty()) {
        LEAFRA_DEBUG_LOG("ERROR", "chunk_document called with empty pages");
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    if (options.chunk_size == 0) {
        LEAFRA_DEBUG_LOG("ERROR", "chunk_document called with zero chunk size");
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    if (options.overlap_percentage < 0.0 || options.overlap_percentage >= 1.0) {
        LEAFRA_DEBUG_LOG("ERROR", "chunk_document called with invalid overlap percentage: " + std::to_string(options.overlap_percentage));
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    auto start_time = debug::timer::now();
    
    try {
        chunks.clear();
        
        // Calculate total document length
        size_t total_length = 0;
        for (const auto& page : pages) {
            total_length += page.length();
        }
        last_total_characters_ = total_length;
        
        LEAFRA_DEBUG_LOG("CHUNKING", "Processing document with " + std::to_string(pages.size()) + 
                        " pages, " + std::to_string(total_length) + " total characters");
        LEAFRA_DEBUG_LOG("OPTIONS", "Chunk size: " + std::to_string(options.chunk_size) + 
                        ", Overlap: " + std::to_string(options.overlap_percentage * 100.0) + "%");
        
        // Store current default options and set new ones temporarily
        ChunkingOptions old_options = default_options_;
        default_options_ = options;
        
        // Determine effective options for chunking
        ChunkingOptions effective_options = options;
        if (options.size_unit == ChunkSizeUnit::CHARACTERS) {
            // Convert character size to approximate token size for the unified method
            size_t approx_tokens = static_cast<size_t>(options.chunk_size / SIMPLE_CHARS_PER_TOKEN); // ~4 chars per token
            if (approx_tokens < 1) approx_tokens = 1;
            effective_options.chunk_size = approx_tokens;
            effective_options.size_unit = ChunkSizeUnit::TOKENS;
            LEAFRA_DEBUG_LOG("CONVERSION", "Converted " + std::to_string(options.chunk_size) + 
                           " characters to " + std::to_string(approx_tokens) + " tokens");
        }
        
        // Combine all pages into a single text with page separators
        auto combine_start = debug::timer::now();
        combined_text_.clear(); // Clear previous document content
        std::vector<size_t> page_starts;
        page_starts.push_back(0);
        
        for (size_t i = 0; i < pages.size(); ++i) {
            combined_text_ += pages[i];
            if (i < pages.size() - 1) {
                combined_text_ += "\n\n"; // Page separator
                page_starts.push_back(combined_text_.length());
            }
        }
        
        auto combine_end = debug::timer::now();
        double combine_ms = debug::timer::elapsed_milliseconds(combine_start, combine_end);
        LEAFRA_DEBUG_LOG("TIMING", "Text combination took " + std::to_string(combine_ms) + "ms");
        
        // Use core chunking method
        auto chunk_start = debug::timer::now();
        std::vector<TextChunk> temp_chunks;
        documentCacher.reinitialize(combined_text_);
        ResultCode result = actual_chunker(combined_text_, effective_options, temp_chunks);
        
        auto chunk_end = debug::timer::now();
        double chunk_ms = debug::timer::elapsed_milliseconds(chunk_start, chunk_end);
        LEAFRA_DEBUG_LOG("TIMING", "Core chunking took " + std::to_string(chunk_ms) + "ms");
        
        if (result != ResultCode::SUCCESS) {
            LEAFRA_DEBUG_LOG("ERROR", "actual_chunker failed with result code: " + std::to_string(static_cast<int>(result)));
            // Restore old options before returning
            default_options_ = old_options;
            return result;
        }
        
        // Update chunk metadata with correct page numbers
        auto metadata_start = debug::timer::now();
        for (auto& chunk : temp_chunks) {
            size_t page_number = 0;
            for (size_t i = 0; i < page_starts.size(); ++i) {
                if (chunk.start_index >= page_starts[i]) {
                    page_number = i;
                } else {
                    break;
                }
            }
            chunk.page_number = page_number;
        }
        
        auto metadata_end = debug::timer::now();
        double metadata_ms = debug::timer::elapsed_milliseconds(metadata_start, metadata_end);
        LEAFRA_DEBUG_LOG("TIMING", "Metadata update took " + std::to_string(metadata_ms) + "ms");
        
        chunks = std::move(temp_chunks);
        
        // Restore old options
        default_options_ = old_options;
        
        // Update statistics
        last_chunk_count_ = chunks.size();
        
        // Log final performance metrics
        if (debug::is_debug_enabled()) {
            auto end_time = debug::timer::now();
            double total_duration_ms = debug::timer::elapsed_milliseconds(start_time, end_time);
            debug::debug_log_performance("chunk_document", total_length, chunks.size(), total_duration_ms);
            
            // Log individual chunk details
            for (size_t i = 0; i < chunks.size() && i < 5; ++i) { // Log first 5 chunks only           
                debug::debug_log_chunking_details("CREATED", i, chunks[i].start_index, chunks[i].end_index, 
                                                 chunks[i].estimated_tokens, effective_options.chunk_size);
            }
            if (chunks.size() > 5) {
                LEAFRA_DEBUG_LOG("CHUNKING", "... and " + std::to_string(chunks.size() - 5) + " more chunks");
            }
        }
        
        return ResultCode::SUCCESS;
        
    } catch (const std::exception& e) {
        LEAFRA_DEBUG_LOG("ERROR", "Exception in chunk_document: " + std::string(e.what()));
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}


//Actual Chunker - Core chunking method
ResultCode LeafraChunker::actual_chunker(const std::string& text,
                                         const ChunkingOptions& options,
                                         std::vector<TextChunk>& chunks) {
    if (text.empty()) {
        chunks.clear();
        return ResultCode::SUCCESS;
    }

    try {
        chunks.clear();
        size_t current_pos = 0;
        size_t target_tokens = options.chunk_size;
        
        // PERFORMANCE OPTIMIZATION: Calculate Unicode length once at the beginning
        // since the text doesn't change throughout the chunking process
        size_t text_unicode_length = documentCacher.get_unicode_length_cached();
        
        // DENSITY SAMPLING: Learn actual text density once at the beginning
        double chars_per_token = sample_text_density(text, options);
        
        while (current_pos < text.length()) {
            // Ensure we start at a word boundary
            if (options.preserve_word_boundaries && current_pos > 0) {
                size_t original_pos = current_pos;
                current_pos = find_next_word_start(text, current_pos);
                // CRITICAL: Ensure the word boundary is also UTF-8 aligned
                current_pos = ensure_utf8_boundary(text, current_pos);
                
                if (current_pos >= text.length()) {
                    break;
                }
            }
            
            // Find the best chunk end that respects word boundaries and target token count
            size_t chunk_end = find_optimal_chunk_end(text, current_pos, target_tokens, options, text_unicode_length, chars_per_token);
            
            // Make sure chunk end is at a word boundary
            if (options.preserve_word_boundaries && chunk_end < text.length()) {
                // Ensure we end at a word boundary (this will move to end of current word)
                chunk_end = find_word_boundary(text, chunk_end, 100);
            }
            
            // Create chunk - but only if it would be non-empty
            TextChunk chunk = create_chunk(text, current_pos, chunk_end, 0, current_pos);
            
            // Skip empty chunks - don't add them to the result
            if (!chunk.content.empty()) {
                // Calculate actual token count
                chunk.estimated_tokens = estimate_token_count(chunk.content, options.token_method);
                chunks.push_back(chunk);
                
                // Calculate next position with proper overlap based on ACTUAL chunk size
                size_t actual_chunk_tokens = chunk.estimated_tokens;
                size_t effective_content_tokens = static_cast<size_t>(actual_chunk_tokens * (1.0 - options.overlap_percentage));
                if (effective_content_tokens < 1) effective_content_tokens = 1;
                
                // Convert back to characters to find next start position
                size_t advance_chars = static_cast<size_t>(std::round(effective_content_tokens * chars_per_token));
                current_pos += advance_chars;
                // CRITICAL: Ensure current_pos is on a UTF-8 character boundary
                current_pos = ensure_utf8_boundary(text, current_pos);
            } else if (chunks.empty() && current_pos < text.length()) {
                // Special case: if we have no chunks yet and there's still text,
                // create a chunk from current position to end of text to avoid empty result
                chunk = create_chunk(text, current_pos, text.length(), 0, current_pos);
                if (!chunk.content.empty()) {
                    chunk.estimated_tokens = estimate_token_count(chunk.content, options.token_method);
                    chunks.push_back(chunk);
                }
                break; // Exit the loop since we've processed all remaining text
            }
            
            // If we've reached the end, break
            if (chunk_end >= text.length()) {
                break;
            }
            
            // Ensure we don't go past the end
            if (current_pos >= text.length()) {
                break;
            }
        }
        
        return ResultCode::SUCCESS;
        
    } catch (const std::exception&) {
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

size_t LeafraChunker::get_chunk_count() const {
    return last_chunk_count_;
}

size_t LeafraChunker::get_total_characters() const {
    return last_total_characters_;
}

void LeafraChunker::reset_statistics() {
    last_chunk_count_ = 0;
    last_total_characters_ = 0;
}

void LeafraChunker::set_default_options(const ChunkingOptions& options) {
    default_options_ = options;
}

const ChunkingOptions& LeafraChunker::get_default_options() const {
    return default_options_;
}

/**
 * Find the nearest word boundary for text chunking
 * @param text UTF-8 text to search
 * @param target_position Starting byte position
 * @param search_window Max bytes to search in each direction
 * @return Byte position of word boundary, or target_position if none found
 */
size_t LeafraChunker::find_word_boundary(const std::string& text, 
                                        size_t target_position, 
                                        size_t search_window) const {
    if (target_position >= text.length()) {
        return text.length();
    }
    
    // If we're at text boundaries, return them
    if (target_position == 0) {
        return 0;
    }
    if (target_position == text.length()) {
        return text.length();
    }
    
    // Check if we're already at a word boundary using Unicode-aware detection
    size_t next_pos;
    UChar32 c = documentCacher.get_unicode_char_at_cached(target_position, next_pos);
    if (c != U_SENTINEL && is_unicode_whitespace(c)) {
        return target_position;
    }
    
    // Try searching backwards for a word boundary
    size_t search_start = (target_position > search_window) ? 
                          target_position - search_window : 0;
    
    size_t byte_pos = target_position;
    while (byte_pos > search_start) {
        // Find previous character position
        size_t prev_pos = 0;
        size_t temp_pos = 0;
        
        while (temp_pos < byte_pos && temp_pos < text.length()) {
            prev_pos = temp_pos;
            size_t next_temp_pos;
            UChar32 temp_c = documentCacher.get_unicode_char_at_cached(temp_pos, next_temp_pos);
            if (temp_c == U_SENTINEL || temp_pos >= byte_pos) break;
            temp_pos = next_temp_pos;
        }
        
        if (prev_pos == 0) break;
        
        UChar32 prev_c = documentCacher.get_unicode_char_at_cached(prev_pos, next_pos);
        if (prev_c != U_SENTINEL && is_unicode_whitespace(prev_c)) {
            // Skip all consecutive whitespace
            do {
                byte_pos = prev_pos;
                if (byte_pos == 0) break;
                size_t before_prev = 0;
                size_t temp = 0;
                while (temp < byte_pos && temp < text.length()) {
                    before_prev = temp;
                    size_t next_temp;
                    UChar32 temp_c = documentCacher.get_unicode_char_at_cached(temp, next_temp);
                    if (temp_c == U_SENTINEL || temp >= byte_pos)
                        break;
                    temp = next_temp;
                }
                prev_c = documentCacher.get_unicode_char_at_cached(before_prev, next_pos);
                prev_pos = before_prev;
            } while (prev_c != U_SENTINEL && is_unicode_whitespace(prev_c));

            return next_pos;
        }
        
        byte_pos = prev_pos;
    }
    
    // If backward search failed, try forward search within window
    size_t search_end = std::min(target_position + search_window, text.length());
    byte_pos = target_position;
    
    while (byte_pos < search_end) {
        UChar32 c = documentCacher.get_unicode_char_at_cached(byte_pos, next_pos);
        if (c == U_SENTINEL) {
            byte_pos = next_pos;
            continue;
        }
        
        if (is_unicode_whitespace(c)) {
            // Move to end of current word (before the whitespace)
            return byte_pos;
        }
        
        byte_pos = next_pos;
    }
    
    // If limited search failed, use the Unicode word boundary function with backward search
    size_t word_boundary = documentCacher.find_word_boundary_helper_for_unicode_cached(target_position, false);
    if (word_boundary != target_position) {
        return word_boundary;
    }
    
    // Last resort: search forward without limit using Unicode word boundary
    return documentCacher.find_word_boundary_helper_for_unicode_cached(target_position, true);
}

/**
 * Creates a TextChunk from the specified range with zero-copy string_view
 * @param text Original text to create chunk from
 * @param start Starting byte position in text
 * @param end Ending byte position in text
 * @param page_number Page number for this chunk
 * @param global_start Global byte position in the entire document
 * @return TextChunk object with content view and metadata
 */
TextChunk LeafraChunker::create_chunk(const std::string& text,
                                     size_t start,
                                     size_t end,
                                     size_t page_number,
                                     size_t global_start) {
    if (start >= text.length() || end > text.length() || start >= end) {
        
        return TextChunk(std::string_view(), start, end, page_number);
    }
    
    size_t content_start = start;
    size_t content_end = end;
    
    // FIRST: Ensure we're at valid UTF-8 character boundaries
    // Align content_start to a valid UTF-8 character boundary
    while (content_start < content_end && content_start < text.length() && 
           (text[content_start] & 0xC0) == 0x80) {
        content_start++; // Skip UTF-8 continuation bytes
    }
    
    // Align content_end to a valid UTF-8 character boundary
    while (content_end > content_start && content_end > 0 &&
           (text[content_end - 1] & 0xC0) == 0x80) {
        content_end--; // Move back to avoid cutting UTF-8 character
    }
    
    // SECOND: Trim leading and trailing whitespace if preserve_word_boundaries is enabled
    if (default_options_.preserve_word_boundaries && content_start < content_end) {
        // Find content start by skipping leading Unicode whitespace
        size_t trimmed_start = content_start;
        while (trimmed_start < content_end) {
            size_t next_pos;
            UChar32 c = documentCacher.get_unicode_char_at_cached(trimmed_start, next_pos);
            if (c == U_SENTINEL || !is_unicode_whitespace(c)) {
                break;
            }
            trimmed_start = next_pos;
        }
        
        // Find content end by working backwards from end, skipping trailing Unicode whitespace
        size_t trimmed_end = content_end;
        while (trimmed_end > trimmed_start) {
            // Find the start of the last character before trimmed_end
            size_t last_char_start = trimmed_start;
            size_t temp_pos = trimmed_start;
            
            while (temp_pos < trimmed_end) {
                last_char_start = temp_pos;
                size_t next_temp_pos;
                UChar32 temp_c = documentCacher.get_unicode_char_at_cached(temp_pos, next_temp_pos);
                if (temp_c == U_SENTINEL || temp_pos >= trimmed_end) break;
                temp_pos = next_temp_pos;
            }
            
            UChar32 c = documentCacher.get_unicode_char_at_cached(last_char_start, temp_pos);
            if (c == U_SENTINEL || !is_unicode_whitespace(c)) {
                break;
            }
            
            trimmed_end = last_char_start;
        }
        
        content_start = trimmed_start;
        content_end = trimmed_end;
    }
    
    // Create string_view directly into the original text - no copying!
    if (content_start < content_end) {
        // Safety checks to prevent string_view corruption
        if (content_start >= text.length() || content_end > text.length()) {
            return TextChunk(std::string_view(), global_start, global_start + (end - start), page_number);
        }
        
        // Validate UTF-8 boundaries one more time
        if (content_start > 0 && (text[content_start] & 0xC0) == 0x80) {           
            // Find the start of the UTF-8 character
            while (content_start > 0 && (text[content_start] & 0xC0) == 0x80) {
                content_start--;
            }
        }
        
        if (content_end > 0 && content_end < text.length() && (text[content_end] & 0xC0) == 0x80) {
            // Move to end of the UTF-8 character
            while (content_end < text.length() && (text[content_end] & 0xC0) == 0x80) {
                content_end++;
            }
        }
        
        // Create zero-copy string_view pointing directly to member variable text
        // This is safe because text parameter references combined_text_ member variable
        std::string_view chunk_view(text.data() + content_start, content_end - content_start);
        return TextChunk(chunk_view, global_start, global_start + (end - start), page_number);
    } else {
        // Empty content after trimming - return empty but valid chunk
        return TextChunk(std::string_view(), global_start, global_start + (end - start), page_number);
    }
}

/**
 * Estimates token count from text using the unified simple approach.
 * 
 * @param text The text to analyze (as string_view for efficiency)
 * @param method The approximation method (only SIMPLE is supported now)
 * @return Estimated number of tokens
 */
size_t LeafraChunker::estimate_token_count(std::string_view text, TokenApproximationMethod method) {
    if (text.empty()) return 0;
    
    // Use unified approach: ~4 characters per token
    return static_cast<size_t>(std::round(text.length() / SIMPLE_CHARS_PER_TOKEN));
}


// Helper methods for improved token-based chunking
/**
 * Find where to end a chunk to match target token count
 * Works in UTF-8 character space internally, converts to/from byte positions
 * @param text UTF-8 text to chunk
 * @param start_pos Starting byte position
 * @param target_tokens Desired number of tokens in chunk
 * @param options Chunking configuration options
 * @return End byte position that produces target token count
 */
size_t LeafraChunker::find_optimal_chunk_end(const std::string& text,
                                            size_t start_pos,
                                            size_t target_tokens,
                                            const ChunkingOptions& options,
                                            size_t text_unicode_length,
                                            double chars_per_token) const {
    if (start_pos >= text.length()) {
        return text.length();
    }
    
    // Convert start byte position to character position for Unicode-aware processing
    size_t start_char_pos = 0;  // UTF-8 character position (not bytes)
    size_t byte_pos = 0;        // Byte position for conversion
    
    // Find character position corresponding to start_pos byte position
    while (byte_pos < start_pos && byte_pos < text.length()) {
        size_t next_pos;
        UChar32 c = documentCacher.get_unicode_char_at_cached(byte_pos, next_pos);
        if (c != U_SENTINEL) {
            start_char_pos++;  // Count UTF-8 characters
        }
        byte_pos = next_pos;
        
        if (next_pos <= byte_pos) byte_pos++; // Safety check
    }
    
    // Start with a conservative character estimate
    size_t estimated_chars = static_cast<size_t>(target_tokens * chars_per_token);
    
    size_t target_end_char_pos = std::min(start_char_pos + estimated_chars, text_unicode_length);
    
    // DIRECT CALCULATION - No iterations needed!
    // Since we know the exact density, calculate the precise character count for target tokens
    size_t precise_chars = static_cast<size_t>(std::round(target_tokens * chars_per_token));
    size_t final_end_char_pos = std::min(start_char_pos + precise_chars, text_unicode_length);
    
    // Convert final character position back to byte position
    size_t final_byte_pos = documentCacher.get_byte_pos_for_char_index_cached(final_end_char_pos);
    
    return std::min(final_byte_pos, text.length());
}

/**
 * Estimate how many characters needed to produce target token count
 * @param text UTF-8 text to sample from
 * @param start_pos Starting byte position for sampling
 * @param target_tokens Desired number of tokens
 * @param method Token approximation method to use
 * @return Estimated character count that produces target tokens
 */
size_t LeafraChunker::estimate_characters_for_tokens(const std::string& text,
                                                    size_t start_pos,
                                                    size_t target_tokens,
                                                    TokenApproximationMethod method,
                                                    size_t text_unicode_length) const {
    if (start_pos >= text.length() || target_tokens == 0) {
        return 0;
    }
    
    // Convert start byte position to character position
    size_t start_char_pos = 0;
    size_t byte_pos = 0;
    
    while (byte_pos < start_pos && byte_pos < text.length()) {
        size_t next_pos;
        UChar32 c = get_unicode_char_at(text, byte_pos, next_pos);
        if (c != U_SENTINEL) {
            start_char_pos++;
        }
        byte_pos = next_pos;
        if (next_pos <= byte_pos) byte_pos++; // Safety check
    }
    
    // Use local token density if we have enough context
    size_t remaining_chars = text_unicode_length - start_char_pos;
    size_t sample_chars = std::min(static_cast<size_t>(100), remaining_chars); // Sample in characters, not bytes
    
    if (sample_chars > 10) {
        std::string sample = get_utf8_substring(text, start_char_pos, sample_chars);
        size_t sample_tokens = estimate_token_count(sample, method);
        
        if (sample_tokens > 0) {
            // Calculate local density and use it
            double chars_per_token = static_cast<double>(sample_chars) / sample_tokens;
            return static_cast<size_t>(target_tokens * chars_per_token);
        }
    }
    
    // Fall back to global estimate
    return tokens_to_characters(target_tokens, method);
}

/**
 * Find the start of the next word using Unicode-aware detection
 * @param text UTF-8 text to search
 * @param pos Starting byte position
 * @return Byte position of next word start, or text length if none found
 */
size_t LeafraChunker::find_next_word_start(const std::string& text, size_t pos) const {
    if (pos >= text.length()) {
        return text.length();
    }
    
    // If we're at the start of text, that's a word start
    if (pos == 0) {
        return 0;
    }
    
    // Check if we're already at the start of a word using Unicode-aware detection
    size_t next_pos;
    UChar32 current_c = documentCacher.get_unicode_char_at_cached(pos, next_pos);
    
    if (current_c != U_SENTINEL && !is_unicode_whitespace(current_c)) {
        // Check if previous character is whitespace
        if (pos > 0) {
            // Find previous character position
            size_t prev_pos = 0;
            size_t temp_pos = 0;
            
            while (temp_pos < pos && temp_pos < text.length()) {
                prev_pos = temp_pos;
                size_t next_temp_pos;
                UChar32 temp_c = documentCacher.get_unicode_char_at_cached(temp_pos, next_temp_pos);
                if (temp_c == U_SENTINEL || temp_pos >= pos) break;
                temp_pos = next_temp_pos;
            }
            
            UChar32 prev_c = documentCacher.get_unicode_char_at_cached(prev_pos, temp_pos);
            if (prev_c != U_SENTINEL && is_unicode_whitespace(prev_c)) {
                // We're at the start of a word
                return pos;
            }
        }
    }
    
    // Advance to the next word boundary
    size_t byte_pos = pos;
    bool in_word = (current_c != U_SENTINEL && !is_unicode_whitespace(current_c));
    
    while (byte_pos < text.length()) {
        UChar32 c = documentCacher.get_unicode_char_at_cached(byte_pos, next_pos);
        if (c == U_SENTINEL) {
            byte_pos = next_pos;
            continue;
        }
        
        bool is_whitespace = is_unicode_whitespace(c);
        
        if (in_word && is_whitespace) {
            // We've reached the end of current word, now skip whitespace
            byte_pos = next_pos;
            break;
        } else if (!in_word && !is_whitespace) {
            // We've found the start of the next word
            return byte_pos;
        }
        
        in_word = !is_whitespace;
        byte_pos = next_pos;
    }
    
    // Skip remaining whitespace to find the start of the next word
    while (byte_pos < text.length()) {
        UChar32 c = documentCacher.get_unicode_char_at_cached(byte_pos, next_pos);
        if (c == U_SENTINEL || !is_unicode_whitespace(c)) {
            break;
        }
        byte_pos = next_pos;
    }
    
    return byte_pos;
}

/**
 * Converts token count to estimated character count.
 * Uses a simple heuristic of ~4 characters per token for all methods.
 * 
 * @param token_count Number of tokens to convert
 * @param method Token approximation method (all use same factor for simplicity)
 * @return Estimated number of characters
 */
size_t LeafraChunker::tokens_to_characters(size_t token_count, TokenApproximationMethod method) {
    if (token_count == 0) return 0;
    
    // Use single factor for all methods - simplicity over micro-optimization
    return static_cast<size_t>(std::round(token_count * SIMPLE_CHARS_PER_TOKEN));
}

/**
 * Sample text density to learn actual characters per token ratio for this specific text
 * @param text The text to sample from
 * @param options Chunking options
 * @return Actual characters per token ratio for this text
 */
double LeafraChunker::sample_text_density(const std::string& text, const ChunkingOptions& options) const {
    if (text.empty()) {
        return SIMPLE_CHARS_PER_TOKEN; // Fall back to default
    }
    
    // Sample multiple segments to get better average density
    const size_t sample_size_chars = 200;  // Sample 200 characters at a time
    const size_t max_samples = 5;          // Take up to 5 samples
    
    size_t text_unicode_length = documentCacher.get_unicode_length_cached();
    size_t available_samples = std::min(max_samples, text_unicode_length / sample_size_chars);
    
    if (available_samples == 0) {
        // Text too short, just sample the whole thing
        size_t tokens = estimate_token_count(text, options.token_method);
        return tokens > 0 ? static_cast<double>(text_unicode_length) / tokens : SIMPLE_CHARS_PER_TOKEN;
    }
    
    double total_chars = 0;
    double total_tokens = 0;
    
    // Take samples from different parts of the text
    for (size_t i = 0; i < available_samples; ++i) {
        size_t sample_start_char = (i * text_unicode_length) / available_samples;
        size_t sample_end_char = std::min(sample_start_char + sample_size_chars, text_unicode_length);
        size_t sample_char_count = sample_end_char - sample_start_char;
        
        if (sample_char_count > 0) {
            std::string sample = documentCacher.get_utf8_substring_cached(sample_start_char, sample_char_count);
            size_t sample_tokens = estimate_token_count(sample, options.token_method);
            
            if (sample_tokens > 0) {
                total_chars += sample_char_count;
                total_tokens += sample_tokens;
            }
        }
    }
    
    if (total_tokens > 0) {
        double density = total_chars / total_tokens;
        
        // Clamp to reasonable bounds (2-8 chars per token)
        density = std::max(2.0, std::min(8.0, density));
        
        LEAFRA_DEBUG_LOG("DENSITY", "Sampled text density: " + std::to_string(density) + " chars/token");
        return density;
    }
    
    return SIMPLE_CHARS_PER_TOKEN; // Fall back to default
}

} // namespace leafra
