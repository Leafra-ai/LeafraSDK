#include "leafra/leafra_chunker.h"
#include <algorithm>
#include <cctype>
#include <cmath>

namespace leafra {

LeafraChunker::LeafraChunker() = default;

LeafraChunker::~LeafraChunker() = default;

ResultCode LeafraChunker::initialize() {
    // Initialize default options
    default_options_ = ChunkingOptions();
    reset_statistics();
    return ResultCode::SUCCESS;
}

ResultCode LeafraChunker::chunk_text(const std::string& text,
                                    size_t chunk_size,
                                    double overlap_percentage,
                                    std::vector<TextChunk>& chunks) {
    // Simple wrapper: Convert single text to single-page document
    std::vector<std::string> pages;
    pages.push_back(text);
    return chunk_document(pages, chunk_size, overlap_percentage, chunks);
}

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
            
            size_t overlap_size = static_cast<size_t>(chunk_size * overlap_percentage);
            size_t step_size = chunk_size - overlap_size;
            size_t current_pos = 0;
            
            while (current_pos < text.length()) {
                size_t chunk_end = std::min(current_pos + chunk_size, text.length());
                
                // Adjust for word boundaries if enabled in default options
                if (default_options_.preserve_word_boundaries && chunk_end < text.length()) {
                    chunk_end = find_word_boundary(text, chunk_end);
                }
                
                // Create chunk
                TextChunk chunk = create_chunk(text, current_pos, chunk_end, 0, current_pos);
                chunks.push_back(chunk);
                
                // Move to next position
                current_pos += step_size;
                
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
        
        // Handle token-based or character-based chunking internally
        if (options.size_unit == ChunkSizeUnit::TOKENS) {
            // Convert token size to character size and do chunking directly
            size_t chunk_size_chars = tokens_to_characters(options.chunk_size, options.token_method);
            
            if (pages.size() == 1) {
                result = chunk_text(pages[0], chunk_size_chars, options.overlap_percentage, chunks);
            } else {
                result = chunk_document(pages, chunk_size_chars, options.overlap_percentage, chunks);
            }
            
            // Update token estimates for each chunk
            if (result == ResultCode::SUCCESS) {
                for (auto& chunk : chunks) {
                    chunk.estimated_tokens = estimate_token_count(chunk.content, options.token_method);
                }
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

ResultCode LeafraChunker::chunk_document_tokens(const std::vector<std::string>& pages,
                                               size_t chunk_size_tokens,
                                               double overlap_percentage,
                                               TokenApproximationMethod method,
                                               std::vector<TextChunk>& chunks) {
    // Legacy wrapper: Create options and delegate to advanced method
    ChunkingOptions options(chunk_size_tokens, overlap_percentage, ChunkSizeUnit::TOKENS, method);
    return chunk_document_advanced(pages, options, chunks);
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
    
    // If we're already at a word boundary, return it
    if (std::isspace(text[target_position]) || std::ispunct(text[target_position])) {
        return target_position;
    }
    
    // Search backwards for a word boundary
    size_t search_start = (target_position > search_window) ? 
                          target_position - search_window : 0;
    
    for (size_t i = target_position; i > search_start; --i) {
        if (std::isspace(text[i]) || std::ispunct(text[i])) {
            return i + 1; // Return position after the separator
        }
    }
    
    // Search forwards for a word boundary
    size_t search_end = std::min(target_position + search_window, text.length());
    
    for (size_t i = target_position; i < search_end; ++i) {
        if (std::isspace(text[i]) || std::ispunct(text[i])) {
            return i;
        }
    }
    
    // If no boundary found, return original position
    return target_position;
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

size_t LeafraChunker::estimate_token_count(const std::string& text, TokenApproximationMethod method) {
    switch (method) {
        case TokenApproximationMethod::SIMPLE:
            return estimate_tokens_simple(text);
        case TokenApproximationMethod::WORD_BASED:
            return estimate_tokens_word_based(text);
        case TokenApproximationMethod::ADVANCED:
            return estimate_tokens_advanced(text);
        default:
            return estimate_tokens_word_based(text); // Default fallback
    }
}

size_t LeafraChunker::tokens_to_characters(size_t token_count, TokenApproximationMethod method) {
    switch (method) {
        case TokenApproximationMethod::SIMPLE:
            return token_count * 4; // 1 token ≈ 4 characters
        case TokenApproximationMethod::WORD_BASED:
            return static_cast<size_t>(token_count / 0.75 * 5); // 1 token ≈ 0.75 words, avg 5 chars/word
        case TokenApproximationMethod::ADVANCED:
            return static_cast<size_t>(token_count * 3.8); // Advanced heuristic average
        default:
            return static_cast<size_t>(token_count / 0.75 * 5); // Default to word-based
    }
}

size_t LeafraChunker::estimate_tokens_simple(const std::string& text) {
    // Simple: 1 token ≈ 4 characters
    return (text.length() + 3) / 4; // Round up
}

size_t LeafraChunker::estimate_tokens_word_based(const std::string& text) {
    // Word-based: 1 token ≈ 0.75 words
    if (text.empty()) return 0;
    
    size_t word_count = 0;
    bool in_word = false;
    
    for (char c : text) {
        if (std::isalnum(c) || c == '\'' || c == '-') {
            if (!in_word) {
                word_count++;
                in_word = true;
            }
        } else {
            in_word = false;
        }
    }
    
    // 1 token ≈ 0.75 words, so tokens = words / 0.75
    return static_cast<size_t>(word_count / 0.75 + 0.5); // Round to nearest
}

size_t LeafraChunker::estimate_tokens_advanced(const std::string& text) {
    // Advanced heuristic based on word length and character types
    if (text.empty()) return 0;
    
    size_t token_count = 0;
    size_t current_word_length = 0;
    bool in_word = false;
    
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];
        
        if (std::isalnum(c) || c == '\'' || c == '-') {
            current_word_length++;
            in_word = true;
        } else {
            if (in_word) {
                // Process completed word
                if (current_word_length <= 3) {
                    token_count += 1; // Short words: 1 token
                } else if (current_word_length <= 6) {
                    token_count += 1; // Medium words: 1 token
                } else if (current_word_length <= 10) {
                    token_count += 2; // Long words: 2 tokens
                } else {
                    token_count += (current_word_length + 4) / 5; // Very long words: ~1 token per 5 chars
                }
                
                current_word_length = 0;
                in_word = false;
            }
            
            // Handle punctuation and special characters
            if (std::ispunct(c)) {
                token_count += 1; // Punctuation is typically 1 token
            }
        }
    }
    
    // Handle final word if text ends with alphanumeric
    if (in_word && current_word_length > 0) {
        if (current_word_length <= 3) {
            token_count += 1;
        } else if (current_word_length <= 6) {
            token_count += 1;
        } else if (current_word_length <= 10) {
            token_count += 2;
        } else {
            token_count += (current_word_length + 4) / 5;
        }
    }
    
    return token_count;
}

} // namespace leafra
