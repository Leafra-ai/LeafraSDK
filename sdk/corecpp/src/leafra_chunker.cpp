#include "leafra/leafra_chunker.h"
#include <algorithm>
#include <cctype>
#include <cmath>

namespace leafra {

// Token-to-character conversion constant
// Based on empirical analysis: ~4 characters per token works well across different content types
constexpr double SIMPLE_CHARS_PER_TOKEN = 4.0;

LeafraChunker::LeafraChunker() = default;

LeafraChunker::~LeafraChunker() = default;

ResultCode LeafraChunker::initialize() {
    // Initialize default options
    default_options_ = ChunkingOptions();
    reset_statistics();
    return ResultCode::SUCCESS;
}

// TEXT CHUNKING
ResultCode LeafraChunker::chunk_text(const std::string& text,
                                    size_t chunk_size,
                                    double overlap_percentage,
                                    std::vector<TextChunk>& chunks) {
    // Simple wrapper: Convert single text to single-page document
    std::vector<std::string> pages;
    pages.push_back(text);
    return chunk_document(pages, chunk_size, overlap_percentage, chunks);
}

ResultCode LeafraChunker::chunk_text_advanced(const std::string& text,
                                             const ChunkingOptions& options,
                                             std::vector<TextChunk>& chunks) {
    // Simple wrapper: Convert single text to single-page document
    std::vector<std::string> pages;
    pages.push_back(text);
    return chunk_document_advanced(pages, options, chunks);
}

ResultCode LeafraChunker::chunk_text_tokens(const std::string& text,
                                           size_t chunk_size_tokens,
                                           double overlap_percentage,
                                           TokenApproximationMethod method,
                                           std::vector<TextChunk>& chunks) {
    // Simple wrapper: Convert single text to single-page document
    std::vector<std::string> pages;
    pages.push_back(text);
    return chunk_document_tokens(pages, chunk_size_tokens, overlap_percentage, method, chunks);
}

// New improved token-based chunking methods
ResultCode LeafraChunker::chunk_text_tokens_improved(const std::string& text,
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
        
        while (current_pos < text.length()) {
            // Ensure we start at a word boundary
            if (options.preserve_word_boundaries && current_pos > 0) {
                current_pos = find_next_word_start(text, current_pos);
                if (current_pos >= text.length()) {
                    break;
                }
            }
            
            // Find the best chunk end that respects word boundaries and target token count
            size_t chunk_end = find_optimal_chunk_end(text, current_pos, target_tokens, options);
            
            // Make sure chunk end is at a word boundary
            if (options.preserve_word_boundaries && chunk_end < text.length()) {
                // Ensure we end at a word boundary (this will move to end of current word)
                chunk_end = find_word_boundary(text, chunk_end, 100);
            }
            
            // Create chunk
            TextChunk chunk = create_chunk(text, current_pos, chunk_end, 0, current_pos);
            
            // Calculate actual token count
            chunk.estimated_tokens = estimate_token_count(chunk.content, options.token_method);
            
            chunks.push_back(chunk);
            
            // Calculate next position with proper overlap
            size_t effective_content_tokens = static_cast<size_t>(target_tokens * (1.0 - options.overlap_percentage));
            if (effective_content_tokens < 1) effective_content_tokens = 1;
            
            // Convert back to characters to find next start position
            size_t advance_chars = estimate_characters_for_tokens(text, current_pos, effective_content_tokens, options.token_method);
            current_pos += advance_chars;
            
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


// DOCUMENT CHUNKING

ResultCode LeafraChunker::chunk_document(const std::vector<std::string>& pages,
                                        size_t chunk_size,
                                        double overlap_percentage,
                                        std::vector<TextChunk>& chunks) {
    if (pages.empty()) {
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    if (chunk_size == 0) {
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    if (overlap_percentage < 0.0 || overlap_percentage >= 1.0) {
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    // Additional validation for empty text in single-page case
    if (pages.size() == 1 && pages[0].empty()) {
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    try {
        chunks.clear();
        
        // Calculate total document length
        size_t total_length = 0;
        for (const auto& page : pages) {
            total_length += page.length();
        }
        last_total_characters_ = total_length;
        
        // Handle single page as special case (no page separators needed)
        if (pages.size() == 1) {
            const std::string& text = pages[0];
            
            // If text is smaller than chunk size, return as single chunk
            if (text.length() <= chunk_size) {
                chunks.emplace_back(text, 0, text.length(), 0);
                last_chunk_count_ = 1;
                return ResultCode::SUCCESS;
            }
            
            // Calculate sizes accounting for overlap properly
            // Goal: Each chunk should be close to chunk_size total (including overlap)
            // If overlap is 20%, then effective new content per chunk should be ~80% of chunk_size
            // But the total chunk size should still be close to chunk_size
            
            // Calculate effective content size (non-overlapping part)
            size_t effective_content_size = static_cast<size_t>(chunk_size * (1.0 - overlap_percentage));
            
            // Ensure minimum effective content size
            if (effective_content_size < 1) {
                effective_content_size = 1;
            }
            
            size_t current_pos = 0;
            
            while (current_pos < text.length()) {
                // Calculate chunk end position
                size_t target_chunk_end = current_pos + chunk_size;
                size_t chunk_end = std::min(target_chunk_end, text.length());
                
                // Adjust for word boundaries if enabled in default options
                if (default_options_.preserve_word_boundaries && chunk_end < text.length()) {
                    // Use a proportional search window (15% of chunk size, min 50, max 300)
                    size_t search_window = std::max(50UL, std::min(300UL, chunk_size / 7));
                    
                    size_t word_boundary = find_word_boundary(text, chunk_end, search_window);
                    
                    // Apply hard limit: don't exceed 150% of target chunk size to prevent extreme overshoots
                    size_t max_chunk_end = current_pos + (chunk_size * 3 / 2); // 150% of chunk size
                    
                    if (word_boundary <= max_chunk_end) {
                        // Word boundary is within acceptable limit
                        chunk_end = word_boundary;
                    } else {
                        // Word boundary exceeds limit, find a closer boundary
                        // Search backwards from max_chunk_end for a word boundary
                        size_t fallback_boundary = find_word_boundary(text, max_chunk_end, search_window);
                        
                        // If fallback is too far back, use original word boundary anyway
                        // (better to have a slightly large chunk than split words)
                        if (fallback_boundary >= current_pos + (chunk_size * 2 / 3)) { // At least 67% of target size
                            chunk_end = fallback_boundary;
                        } else {
                            // Original word boundary is best option even if large
                            chunk_end = word_boundary;
                        }
                    }
                    
                    // Final safety check against text length
                    chunk_end = std::min(chunk_end, text.length());
                }
                
                // Create chunk
                TextChunk chunk = create_chunk(text, current_pos, chunk_end, 0, current_pos);
                chunks.push_back(chunk);
                
                // Move to next position by effective content size (this creates the overlap)
                current_pos += effective_content_size;
                
                // Also adjust the start position to a word boundary if needed
                if (default_options_.preserve_word_boundaries && current_pos < text.length()) {
                    // Check if we're starting in the middle of a word (space-only approach)
                    if (current_pos > 0 && !std::isspace(text[current_pos]) && !std::isspace(text[current_pos - 1])) {
                        // We're in the middle of a word, find the next space
                        while (current_pos < text.length() && !std::isspace(text[current_pos])) {
                            current_pos++;
                        }
                        // Skip any following whitespace to start of next word
                        while (current_pos < text.length() && std::isspace(text[current_pos])) {
                            current_pos++;
                        }
                    }
                }
                
                // If we've reached the end, break
                if (chunk_end == text.length()) {
                    break;
                }
            }
            
            last_chunk_count_ = chunks.size();
            return ResultCode::SUCCESS;
        }
        
        // For multi-page documents, concatenate with page separators
        std::string combined_text;
        std::vector<size_t> page_starts;
        page_starts.push_back(0);
        
        for (size_t i = 0; i < pages.size(); ++i) {
            combined_text += pages[i];
            if (i < pages.size() - 1) {
                combined_text += "\n\n"; // Page separator
                page_starts.push_back(combined_text.length());
            }
        }
        
        // Use the single-page logic on the combined text
        std::vector<std::string> combined_pages;
        combined_pages.push_back(combined_text);
        std::vector<TextChunk> temp_chunks;
        ResultCode result = chunk_document(combined_pages, chunk_size, overlap_percentage, temp_chunks);
        
        if (result != ResultCode::SUCCESS) {
            return result;
        }
        
        // Update chunk metadata with correct page numbers
        for (auto& chunk : temp_chunks) {
            // Find which page this chunk belongs to
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
        
        chunks = std::move(temp_chunks);
        last_chunk_count_ = chunks.size();
        return ResultCode::SUCCESS;
        
    } catch (const std::exception&) {
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

ResultCode LeafraChunker::chunk_document_advanced(const std::vector<std::string>& pages,
                                                 const ChunkingOptions& options,
                                                 std::vector<TextChunk>& chunks) {
    if (pages.empty()) {
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    if (options.chunk_size == 0) {
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    if (options.overlap_percentage < 0.0 || options.overlap_percentage >= 1.0) {
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    try {
        chunks.clear();
        
        // Calculate total document length
        size_t total_length = 0;
        for (const auto& page : pages) {
            total_length += page.length();
        }
        last_total_characters_ = total_length;
        
        // Store current default options and set new ones temporarily
        ChunkingOptions old_options = default_options_;
        default_options_ = options;
        
        ResultCode result;
        
        // For token-based chunking, we need more sophisticated handling
        if (options.size_unit == ChunkSizeUnit::TOKENS) {
            // Use a more accurate initial size estimate that accounts for word boundaries
            size_t chunk_size_chars = tokens_to_characters(options.chunk_size, options.token_method);
            
            // Add buffer for word boundary adjustments (20% extra to account for boundary shifts)
            chunk_size_chars = static_cast<size_t>(chunk_size_chars * 1.2);
            
            if (pages.size() == 1) {
                result = chunk_text_tokens_improved(pages[0], options, chunks);
            } else {
                result = chunk_document_tokens_improved(pages, options, chunks);
            }
        } else {
            // Use character-based chunking (default)
            if (pages.size() == 1) {
                result = chunk_text(pages[0], options.chunk_size, options.overlap_percentage, chunks);
            } else {
                result = chunk_document(pages, options.chunk_size, options.overlap_percentage, chunks);
            }
        }
        
        // Restore old options
        default_options_ = old_options;
        
        return result;
        
    } catch (const std::exception&) {
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}



ResultCode LeafraChunker::chunk_document_tokens(const std::vector<std::string>& pages,
                                               size_t chunk_size_tokens,
                                               double overlap_percentage,
                                               TokenApproximationMethod method,
                                               std::vector<TextChunk>& chunks) {
    // Legacy wrapper: Create options and delegate to advanced method
    ChunkingOptions options(chunk_size_tokens, overlap_percentage, ChunkSizeUnit::TOKENS, method);
    return chunk_document_advanced(pages, options, chunks);
}

ResultCode LeafraChunker::chunk_document_tokens_improved(const std::vector<std::string>& pages,
                                                        const ChunkingOptions& options,
                                                        std::vector<TextChunk>& chunks) {
    // For multi-page documents, use the same approach as before but with improved chunking
    std::string combined_text;
    std::vector<size_t> page_starts;
    page_starts.push_back(0);
    
    for (size_t i = 0; i < pages.size(); ++i) {
        combined_text += pages[i];
        if (i < pages.size() - 1) {
            combined_text += "\n\n"; // Page separator
            page_starts.push_back(combined_text.length());
        }
    }
    
    // Use improved single-text chunking
    std::vector<TextChunk> temp_chunks;
    ResultCode result = chunk_text_tokens_improved(combined_text, options, temp_chunks);
    
    if (result != ResultCode::SUCCESS) {
        return result;
    }
    
    // Update chunk metadata with correct page numbers
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
    
    chunks = std::move(temp_chunks);
    return ResultCode::SUCCESS;
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

size_t LeafraChunker::find_word_boundary(const std::string& text, 
                                        size_t target_position, 
                                        size_t search_window) const {
    if (target_position >= text.length()) {
        return text.length();
    }
    
    // If we're already at a space or text boundary, return it
    if (target_position == 0 || target_position == text.length() || 
        std::isspace(text[target_position])) {
        return target_position;
    }
    
    // We're in the middle of a word, find the next space
    
    // First, try searching backwards for a space
    size_t search_start = (target_position > search_window) ? 
                          target_position - search_window : 0;
    
    for (size_t i = target_position; i > search_start; --i) {
        if (i == 0 || std::isspace(text[i])) {
            // Found space - return position after the space (or 0)
            if (i == 0) return 0;
            
            // Skip past whitespace to find actual word start
            size_t word_start = i + 1;
            while (word_start < text.length() && std::isspace(text[word_start])) {
                word_start++;
            }
            return word_start;
        }
    }
    
    // If backward search failed, try forward search
    size_t search_end = std::min(target_position + search_window, text.length());
    
    for (size_t i = target_position; i < search_end; ++i) {
        if (i == text.length() - 1 || std::isspace(text[i + 1])) {
            // Move to end of current word (after the last non-space character)
            return i + 1;
        }
    }
    
    // If limited search failed, do an exhaustive backward search to avoid word splitting
    for (size_t i = target_position; i > 0; --i) {
        if (std::isspace(text[i])) {
            // Skip past whitespace to find actual word start
            size_t word_start = i + 1;
            while (word_start < text.length() && std::isspace(text[word_start])) {
                word_start++;
            }
            return word_start;
        }
    }
    
    // If all else fails, search forward without limit
    for (size_t i = target_position; i < text.length(); ++i) {
        if (std::isspace(text[i])) {
            return i;
        }
    }
    
    // Absolute last resort: end of text
    return text.length();
}

TextChunk LeafraChunker::create_chunk(const std::string& text,
                                     size_t start,
                                     size_t end,
                                     size_t page_number,
                                     size_t global_start) const {
    if (start >= text.length() || end > text.length() || start >= end) {
        return TextChunk("", start, end, page_number);
    }
    
    std::string chunk_content = text.substr(start, end - start);
    
    // Trim leading and trailing whitespace if preserve_word_boundaries is enabled
    if (default_options_.preserve_word_boundaries) {
        size_t content_start = 0;
        size_t content_end = chunk_content.length();
        
        // Trim leading whitespace
        while (content_start < content_end && std::isspace(chunk_content[content_start])) {
            ++content_start;
        }
        
        // Trim trailing whitespace
        while (content_end > content_start && std::isspace(chunk_content[content_end - 1])) {
            --content_end;
        }
        
        if (content_start < content_end) {
            chunk_content = chunk_content.substr(content_start, content_end - content_start);
        } else {
            chunk_content.clear();
        }
    }
    
    return TextChunk(chunk_content, global_start, global_start + (end - start), page_number);
}

/**
 * Estimates token count from text using the unified simple approach.
 * 
 * @param text The text to analyze
 * @param method The approximation method (only SIMPLE is supported now)
 * @return Estimated number of tokens
 */
size_t LeafraChunker::estimate_token_count(const std::string& text, TokenApproximationMethod method) {
    if (text.empty()) return 0;
    
    // Use unified approach: ~4 characters per token
    return static_cast<size_t>(std::round(text.length() / SIMPLE_CHARS_PER_TOKEN));
}





// Helper methods for improved token-based chunking
size_t LeafraChunker::find_optimal_chunk_end(const std::string& text,
                                            size_t start_pos,
                                            size_t target_tokens,
                                            const ChunkingOptions& options) const {
    if (start_pos >= text.length()) {
        return text.length();
    }
    
    // Start with a more conservative character estimate
    size_t estimated_chars = tokens_to_characters(target_tokens, options.token_method);
    // Reduce initial estimate to avoid overshooting
    estimated_chars = static_cast<size_t>(estimated_chars * 0.8);
    size_t initial_end = std::min(start_pos + estimated_chars, text.length());
    
    // If we're at the end, return it
    if (initial_end == text.length()) {
        return text.length();
    }
    
    // Iteratively adjust to get closer to target token count
    size_t current_end = initial_end;
    size_t iterations = 0;
    const size_t max_iterations = 8; // More iterations for better accuracy
    
    while (iterations < max_iterations) {
        // Extract current chunk content
        std::string chunk_content = text.substr(start_pos, current_end - start_pos);
        size_t actual_tokens = estimate_token_count(chunk_content, options.token_method);
        
        // Check if we're close enough (within 8% of target for better accuracy)
        double token_ratio = static_cast<double>(actual_tokens) / target_tokens;
        if (token_ratio >= 0.92 && token_ratio <= 1.08) {
            break; // Good enough
        }
        
        // Adjust position more carefully
        if (actual_tokens < target_tokens * 0.92) {
            // Too few tokens, extend chunk more conservatively
            size_t deficit_tokens = target_tokens - actual_tokens;
            size_t chars_needed = tokens_to_characters(deficit_tokens, options.token_method);
            // Be more conservative in extension
            chars_needed = static_cast<size_t>(chars_needed * 0.7);
            current_end = std::min(current_end + chars_needed, text.length());
        } else if (actual_tokens > target_tokens * 1.08) {
            // Too many tokens, shrink chunk
            size_t excess_tokens = actual_tokens - target_tokens;
            size_t chars_to_remove = tokens_to_characters(excess_tokens, options.token_method);
            current_end = (current_end > chars_to_remove) ? current_end - chars_to_remove : start_pos + 1;
        }
        
        // Ensure we don't go past the end or before start
        current_end = std::max(start_pos + 1, std::min(current_end, text.length()));
        
        iterations++;
    }
    
    // Don't let chunks get extremely large (safety limit)
    size_t max_chars = tokens_to_characters(target_tokens, options.token_method) * 2;
    if (current_end - start_pos > max_chars) {
        current_end = start_pos + max_chars;
    }
    
    return std::min(current_end, text.length());
}

size_t LeafraChunker::estimate_characters_for_tokens(const std::string& text,
                                                    size_t start_pos,
                                                    size_t target_tokens,
                                                    TokenApproximationMethod method) const {
    if (start_pos >= text.length() || target_tokens == 0) {
        return 0;
    }
    
    // Use local token density if we have enough context
    size_t sample_size = std::min(500UL, text.length() - start_pos);
    if (sample_size > 50) {
        std::string sample = text.substr(start_pos, sample_size);
        size_t sample_tokens = estimate_token_count(sample, method);
        
        if (sample_tokens > 0) {
            // Calculate local density and use it
            double chars_per_token = static_cast<double>(sample_size) / sample_tokens;
            return static_cast<size_t>(target_tokens * chars_per_token);
        }
    }
    
    // Fall back to global estimate
    return tokens_to_characters(target_tokens, method);
}

size_t LeafraChunker::find_next_word_start(const std::string& text, size_t pos) const {
    if (pos >= text.length()) {
        return text.length();
    }
    
    // If we're at the start of text, that's a word start
    if (pos == 0) {
        return 0;
    }
    
    // If we're already at the start of a word (after a space), return current position
    if (!std::isspace(text[pos]) && (pos == 0 || std::isspace(text[pos - 1]))) {
        return pos;
    }
    
    // If we're in the middle of a word, advance to the next space
    while (pos < text.length() && !std::isspace(text[pos])) {
        pos++;
    }
    
    // Skip whitespace to find the start of the next word
    while (pos < text.length() && std::isspace(text[pos])) {
        pos++;
    }
    
    return pos;
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

} // namespace leafra
