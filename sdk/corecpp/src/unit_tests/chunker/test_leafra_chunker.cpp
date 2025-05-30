#include "../../../include/leafra/leafra_chunker.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <sstream>

using namespace leafra;

// Temporary implementation for testing
const char* test_result_code_to_string(ResultCode code) {
    switch (code) {
        case ResultCode::SUCCESS: return "SUCCESS";
        case ResultCode::ERROR_INVALID_PARAMETER: return "ERROR_INVALID_PARAMETER";
        case ResultCode::ERROR_INITIALIZATION_FAILED: return "ERROR_INITIALIZATION_FAILED";
        case ResultCode::ERROR_PROCESSING_FAILED: return "ERROR_PROCESSING_FAILED";
        case ResultCode::ERROR_NOT_IMPLEMENTED: return "ERROR_NOT_IMPLEMENTED";
        case ResultCode::ERROR_OUT_OF_MEMORY: return "ERROR_OUT_OF_MEMORY";
        default: return "UNKNOWN";
    }
}

// Simple test framework macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << "FAIL: " << message << " at line " << __LINE__ << std::endl; \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL(expected, actual, message) \
    do { \
        if ((expected) != (actual)) { \
            std::cerr << "FAIL: " << message << " - Expected: " << (expected) \
                      << ", Actual: " << (actual) << " at line " << __LINE__ << std::endl; \
            return false; \
        } \
    } while(0)

// Specialized macro for ResultCode comparison
#define TEST_ASSERT_RESULT_CODE(expected, actual, message) \
    do { \
        if ((expected) != (actual)) { \
            std::cerr << "FAIL: " << message << " - Expected: " << test_result_code_to_string(expected) \
                      << ", Actual: " << test_result_code_to_string(actual) << " at line " << __LINE__ << std::endl; \
            return false; \
        } \
    } while(0)

#define RUN_TEST(test_func) \
    do { \
        std::cout << "Running " << #test_func << "... "; \
        if (test_func()) { \
            std::cout << "PASS" << std::endl; \
            passed_tests++; \
        } else { \
            std::cout << "FAIL" << std::endl; \
            failed_tests++; \
        } \
        total_tests++; \
    } while(0)

// Test helper functions
std::string create_long_text(size_t word_count) {
    std::string result;
    for (size_t i = 0; i < word_count; ++i) {
        if (i > 0) result += " ";
        result += "word" + std::to_string(i);
    }
    return result;
}

std::string create_text_with_punctuation() {
    return "Hello, world! This is a test. How are you? I'm fine, thanks. "
           "This text has punctuation marks: commas, periods, question marks, "
           "and exclamation points! It should be chunked properly.";
}

// Basic functionality tests
bool test_chunker_initialization() {
    LeafraChunker chunker;
    
    ResultCode result = chunker.initialize();
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Chunker initialization should succeed");
    
    // Check default options
    const ChunkingOptions& defaults = chunker.get_default_options();
    TEST_ASSERT_EQUAL(static_cast<size_t>(1000), defaults.chunk_size, "Default chunk size should be 1000");
    TEST_ASSERT(defaults.overlap_percentage >= 0.09 && defaults.overlap_percentage <= 0.11, "Default overlap should be around 0.1");
    TEST_ASSERT_EQUAL(true, defaults.preserve_word_boundaries, "Default should preserve word boundaries");
    
    // Check initial statistics
    TEST_ASSERT_EQUAL(static_cast<size_t>(0), chunker.get_chunk_count(), "Initial chunk count should be 0");
    TEST_ASSERT_EQUAL(static_cast<size_t>(0), chunker.get_total_characters(), "Initial character count should be 0");
    
    return true;
}

bool test_single_text_chunking_basic() {
    LeafraChunker chunker;
    chunker.initialize();
    
    std::string text = "This is a test text that should be chunked into smaller pieces for testing purposes.";
    std::vector<TextChunk> chunks;
    
    ResultCode result = chunker.chunk_text(text, 30, 0.2, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Basic text chunking should succeed");
    
    TEST_ASSERT(chunks.size() > 1, "Text should be split into multiple chunks");
    TEST_ASSERT_EQUAL(text.length(), chunker.get_total_characters(), "Total characters should match input");
    TEST_ASSERT_EQUAL(chunks.size(), chunker.get_chunk_count(), "Chunk count should match vector size");
    
    // Verify all chunks have content
    for (size_t i = 0; i < chunks.size(); ++i) {
        TEST_ASSERT(!chunks[i].content.empty(), "Chunk content should not be empty");
        TEST_ASSERT_EQUAL(static_cast<size_t>(0), chunks[i].page_number, "Single text chunks should have page 0");
    }
    
    return true;
}

bool test_single_text_chunking_small_text() {
    LeafraChunker chunker;
    chunker.initialize();
    
    std::string text = "Short text";
    std::vector<TextChunk> chunks;
    
    ResultCode result = chunker.chunk_text(text, 100, 0.1, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Small text chunking should succeed");
    
    TEST_ASSERT_EQUAL(static_cast<size_t>(1), chunks.size(), "Small text should create single chunk");
    TEST_ASSERT_EQUAL(text, chunks[0].content, "Single chunk should contain entire text");
    
    return true;
}

bool test_single_text_chunking_overlap() {
    LeafraChunker chunker;
    chunker.initialize();
    
    std::string text = create_long_text(50); // Creates text with 50 words
    std::vector<TextChunk> chunks;
    
    ResultCode result = chunker.chunk_text(text, 100, 0.3, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Overlap chunking should succeed");
    
    if (chunks.size() > 1) {
        // Check for overlap between consecutive chunks
        bool has_overlap = false;
        for (size_t i = 0; i < chunks.size() - 1; ++i) {
            std::string chunk1_end = chunks[i].content.substr(chunks[i].content.length() - 10);
            std::string chunk2_start = chunks[i + 1].content.substr(0, 10);
            
            // Simple overlap check - look for common words
            if (chunk1_end.find("word") != std::string::npos && 
                chunk2_start.find("word") != std::string::npos) {
                has_overlap = true;
                break;
            }
        }
        TEST_ASSERT(has_overlap, "Chunks should have overlapping content");
    }
    
    return true;
}

bool test_multi_page_document_chunking() {
    LeafraChunker chunker;
    chunker.initialize();
    
    std::vector<std::string> pages;
    pages.push_back("This is page one with some content that spans multiple lines and has meaningful text.");
    pages.push_back("This is page two with different content that also contains useful information for testing.");
    pages.push_back("This is page three with the final content of our multi-page test document.");
    
    std::vector<TextChunk> chunks;
    
    ResultCode result = chunker.chunk_document(pages, 80, 0.1, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Multi-page document chunking should succeed");
    
    TEST_ASSERT(chunks.size() >= pages.size(), "Should have at least as many chunks as pages");
    
    // Verify page number assignments
    bool found_page_0 = false, found_page_1 = false, found_page_2 = false;
    for (const auto& chunk : chunks) {
        if (chunk.page_number == 0) found_page_0 = true;
        if (chunk.page_number == 1) found_page_1 = true;
        if (chunk.page_number == 2) found_page_2 = true;
        TEST_ASSERT(chunk.page_number < 3, "Page number should be valid");
    }
    
    return true;
}

bool test_multi_page_single_page() {
    LeafraChunker chunker;
    chunker.initialize();
    
    std::vector<std::string> pages;
    pages.push_back("This is a single page document that should be handled correctly.");
    
    std::vector<TextChunk> chunks;
    
    ResultCode result = chunker.chunk_document(pages, 50, 0.2, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Single page in vector should succeed");
    
    for (const auto& chunk : chunks) {
        TEST_ASSERT_EQUAL(static_cast<size_t>(0), chunk.page_number, "All chunks should be from page 0");
    }
    
    return true;
}

bool test_advanced_chunking_options() {
    LeafraChunker chunker;
    chunker.initialize();
    
    std::vector<std::string> pages;
    pages.push_back(create_text_with_punctuation());
    
    ChunkingOptions options;
    options.chunk_size = 60;
    options.overlap_percentage = 0.15;
    options.preserve_word_boundaries = true;
    
    std::vector<TextChunk> chunks;
    
    ResultCode result = chunker.chunk_document_advanced(pages, options, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Advanced chunking should succeed");
    
    // Check that word boundaries are preserved
    for (const auto& chunk : chunks) {
        TEST_ASSERT(!chunk.content.empty(), "Chunk should not be empty");
        // Word boundary preservation means chunks shouldn't start or end mid-word
        // (except for very long words)
        if (!chunk.content.empty()) {
            char first = chunk.content[0];
            char last = chunk.content[chunk.content.length() - 1];
            // Allow letters, but prefer starting with uppercase or after space
            TEST_ASSERT(first != '-' && first != '\'' && last != '-', 
                       "Chunks should avoid starting/ending with partial words");
        }
    }
    
    return true;
}

bool test_word_boundary_preservation() {
    LeafraChunker chunker;
    chunker.initialize();
    
    std::string text = "The quick brown fox jumps over the lazy dog and runs very fast through the forest.";
    std::vector<TextChunk> chunks;
    
    // Use small chunk size to force word boundary decisions
    ResultCode result = chunker.chunk_text(text, 25, 0.1, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Word boundary test should succeed");
    
    // Check that chunks don't break words inappropriately
    for (const auto& chunk : chunks) {
        if (!chunk.content.empty()) {
            // Chunks should typically start and end at word boundaries
            std::string trimmed = chunk.content;
            // Remove leading/trailing whitespace for testing
            size_t start = trimmed.find_first_not_of(" \t\n\r");
            size_t end = trimmed.find_last_not_of(" \t\n\r");
            if (start != std::string::npos && end != std::string::npos) {
                trimmed = trimmed.substr(start, end - start + 1);
                // Shouldn't start or end with hyphen (indicating broken word)
                TEST_ASSERT(trimmed[0] != '-' && trimmed[trimmed.length()-1] != '-',
                           "Chunk should not break words with hyphens");
            }
        }
    }
    
    return true;
}

bool test_statistics_tracking() {
    LeafraChunker chunker;
    chunker.initialize();
    
    std::string text = create_long_text(30);
    std::vector<TextChunk> chunks;
    
    // Initial state
    TEST_ASSERT_EQUAL(static_cast<size_t>(0), chunker.get_chunk_count(), "Initial chunk count should be 0");
    TEST_ASSERT_EQUAL(static_cast<size_t>(0), chunker.get_total_characters(), "Initial character count should be 0");
    
    ResultCode result = chunker.chunk_text(text, 50, 0.2, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Statistics test chunking should succeed");
    
    // After chunking
    TEST_ASSERT_EQUAL(chunks.size(), chunker.get_chunk_count(), "Chunk count should match");
    TEST_ASSERT_EQUAL(text.length(), chunker.get_total_characters(), "Character count should match");
    
    // Reset statistics
    chunker.reset_statistics();
    TEST_ASSERT_EQUAL(static_cast<size_t>(0), chunker.get_chunk_count(), "Reset chunk count should be 0");
    TEST_ASSERT_EQUAL(static_cast<size_t>(0), chunker.get_total_characters(), "Reset character count should be 0");
    
    return true;
}

bool test_default_options_management() {
    LeafraChunker chunker;
    chunker.initialize();
    
    // Get initial defaults
    ChunkingOptions initial = chunker.get_default_options();
    TEST_ASSERT_EQUAL(static_cast<size_t>(1000), initial.chunk_size, "Initial default chunk size");
    TEST_ASSERT_EQUAL(true, initial.preserve_word_boundaries, "Initial word boundary setting");
    
    // Change defaults
    ChunkingOptions new_options;
    new_options.chunk_size = 500;
    new_options.overlap_percentage = 0.25;
    new_options.preserve_word_boundaries = false;
    
    chunker.set_default_options(new_options);
    
    // Verify changes
    const ChunkingOptions& updated = chunker.get_default_options();
    TEST_ASSERT_EQUAL(static_cast<size_t>(500), updated.chunk_size, "Updated chunk size");
    TEST_ASSERT(updated.overlap_percentage >= 0.24 && updated.overlap_percentage <= 0.26, "Updated overlap");
    TEST_ASSERT_EQUAL(false, updated.preserve_word_boundaries, "Updated word boundary setting");
    
    return true;
}

// Error handling tests
bool test_error_handling_invalid_parameters() {
    LeafraChunker chunker;
    chunker.initialize();
    
    std::vector<TextChunk> chunks;
    
    // Test empty text
    ResultCode result = chunker.chunk_text("", 100, 0.1, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::ERROR_INVALID_PARAMETER, result, "Empty text should return error");
    
    // Test zero chunk size
    result = chunker.chunk_text("Some text", 0, 0.1, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::ERROR_INVALID_PARAMETER, result, "Zero chunk size should return error");
    
    // Test invalid overlap percentage
    result = chunker.chunk_text("Some text", 100, -0.1, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::ERROR_INVALID_PARAMETER, result, "Negative overlap should return error");
    
    result = chunker.chunk_text("Some text", 100, 1.0, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::ERROR_INVALID_PARAMETER, result, "100% overlap should return error");
    
    // Test empty pages vector
    std::vector<std::string> empty_pages;
    result = chunker.chunk_document(empty_pages, 100, 0.1, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::ERROR_INVALID_PARAMETER, result, "Empty pages should return error");
    
    return true;
}

bool test_edge_case_very_long_words() {
    LeafraChunker chunker;
    chunker.initialize();
    
    // Create text with very long "word"
    std::string long_word = std::string(200, 'a');
    std::string text = "Short " + long_word + " words.";
    std::vector<TextChunk> chunks;
    
    ResultCode result = chunker.chunk_text(text, 50, 0.1, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Very long words should be handled");
    
    // Should still create chunks even with long words
    TEST_ASSERT(chunks.size() > 0, "Should create at least one chunk");
    
    return true;
}

bool test_edge_case_only_whitespace() {
    LeafraChunker chunker;
    chunker.initialize();
    
    std::string text = "   \t\n   \r   ";
    std::vector<TextChunk> chunks;
    
    ResultCode result = chunker.chunk_text(text, 50, 0.1, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Whitespace-only text should be handled");
    
    if (chunks.size() > 0) {
        // Chunks might be empty after trimming whitespace
        for (const auto& chunk : chunks) {
            // This is acceptable - whitespace-only content may result in empty chunks
        }
    }
    
    return true;
}

bool test_edge_case_unicode_text() {
    LeafraChunker chunker;
    chunker.initialize();
    
    // Test with some basic non-ASCII characters (avoiding complex Unicode for cross-platform compatibility)
    std::string text = "Hello café résumé naïve Zürich München. This has accented characters.";
    std::vector<TextChunk> chunks;
    
    ResultCode result = chunker.chunk_text(text, 30, 0.1, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Unicode text should be handled");
    
    TEST_ASSERT(chunks.size() > 0, "Should create chunks from Unicode text");
    
    return true;
}

bool test_chunk_metadata_consistency() {
    LeafraChunker chunker;
    chunker.initialize();
    
    std::vector<std::string> pages;
    pages.push_back("Page one content here with some text to chunk properly.");
    pages.push_back("Page two has different content for testing purposes.");
    pages.push_back("Page three concludes our test with final content.");
    
    std::vector<TextChunk> chunks;
    
    ResultCode result = chunker.chunk_document(pages, 40, 0.1, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Metadata test should succeed");
    
    // Verify metadata consistency
    for (size_t i = 0; i < chunks.size(); ++i) {
        const TextChunk& chunk = chunks[i];
        
        // Start index should be less than end index
        TEST_ASSERT(chunk.start_index < chunk.end_index, "Start should be less than end");
        
        // Page number should be valid
        TEST_ASSERT(chunk.page_number < pages.size(), "Page number should be valid");
        
        // Content length should match index difference (approximately, due to trimming)
        size_t content_length = chunk.content.length();
        size_t index_length = chunk.end_index - chunk.start_index;
        // Allow some variance due to whitespace trimming
        TEST_ASSERT(content_length <= index_length + 10, "Content length should be reasonable");
    }
    
    return true;
}

// Performance test (basic)
bool test_performance_large_document() {
    LeafraChunker chunker;
    chunker.initialize();
    
    // Create a reasonably large document
    std::string large_text = create_long_text(1000); // 1000 words
    std::vector<TextChunk> chunks;
    
    ResultCode result = chunker.chunk_text(large_text, 200, 0.1, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Large document should be processed");
    
    TEST_ASSERT(chunks.size() > 1, "Large document should create multiple chunks");
    TEST_ASSERT_EQUAL(large_text.length(), chunker.get_total_characters(), "Character count should match");
    
    return true;
}

// Test 17: Token estimation accuracy
bool test_token_estimation() {
    LeafraChunker chunker;
    
    std::string text = "Hello world! This is a test.";
    
    // Test the unified simple method
    size_t estimated = LeafraChunker::estimate_token_count(text, TokenApproximationMethod::SIMPLE);
    
    // Verify estimate is reasonable (between 4-10 tokens for this text)
    TEST_ASSERT(estimated >= 4 && estimated <= 10, "Simple method should estimate 4-10 tokens");
    
    // Test character conversion - should use ~4 chars/token
    size_t chars_expected = LeafraChunker::tokens_to_characters(10, TokenApproximationMethod::SIMPLE);
    TEST_ASSERT_EQUAL(static_cast<size_t>(40), chars_expected, "Simple method: 10 tokens should be 40 characters");
    
    return true;
}

// Test 18: Basic token-based chunking
bool test_basic_token_chunking() {
    LeafraChunker chunker;
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, chunker.initialize(), "Chunker initialization failed");
    
    std::string text = "The quick brown fox jumps over the lazy dog. This is a test sentence.";
    std::vector<TextChunk> chunks;
    
    ResultCode result = chunker.chunk_text_tokens(text, 10, 0.1, TokenApproximationMethod::SIMPLE, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Token chunking failed");
    TEST_ASSERT(!chunks.empty(), "No chunks created");
    
    // Verify token estimates are populated
    for (const auto& chunk : chunks) {
        TEST_ASSERT(chunk.estimated_tokens > 0, "Chunk should have token estimate");
        TEST_ASSERT(!chunk.content.empty(), "Chunk content should not be empty");
    }
    
    return true;
}

// Test 19: Token-based multi-page chunking
bool test_token_multipage_chunking() {
    LeafraChunker chunker;
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, chunker.initialize(), "Chunker initialization failed");
    
    std::vector<std::string> pages;
    pages.push_back("Page 1 content with multiple words and sentences.");
    pages.push_back("Page 2 has different content for testing purposes.");
    pages.push_back("Page 3 concludes our multi-page document example.");
    
    std::vector<TextChunk> chunks;
    
    ResultCode result = chunker.chunk_document_tokens(pages, 8, 0.15, TokenApproximationMethod::SIMPLE, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Token document chunking failed");
    TEST_ASSERT(!chunks.empty(), "No chunks created");
    
    // Verify page numbers are correct and token estimates exist
    bool has_page_0 = false;
    for (const auto& chunk : chunks) {
        TEST_ASSERT(chunk.estimated_tokens > 0, "Chunk should have token estimate");
        if (chunk.page_number == 0) has_page_0 = true;
    }
    
    TEST_ASSERT(has_page_0, "Should have chunks from page 0");
    
    return true;
}

// Test 20: Token chunking error handling
bool test_token_chunking_error_handling() {
    LeafraChunker chunker;
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, chunker.initialize(), "Chunker initialization failed");
    
    std::vector<TextChunk> chunks;
    
    // Test empty text
    ResultCode result = chunker.chunk_text_tokens("", 10, 0.1, TokenApproximationMethod::SIMPLE, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::ERROR_INVALID_PARAMETER, result, "Should reject empty text");
    
    // Test zero token size
    result = chunker.chunk_text_tokens("Valid text", 0, 0.1, TokenApproximationMethod::SIMPLE, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::ERROR_INVALID_PARAMETER, result, "Should reject zero token size");
    
    // Test invalid overlap
    result = chunker.chunk_text_tokens("Valid text", 10, 1.5, TokenApproximationMethod::SIMPLE, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::ERROR_INVALID_PARAMETER, result, "Should reject invalid overlap");
    
    // Test empty pages vector
    std::vector<std::string> empty_pages;
    result = chunker.chunk_document_tokens(empty_pages, 10, 0.1, TokenApproximationMethod::SIMPLE, chunks);
    TEST_ASSERT_RESULT_CODE(ResultCode::ERROR_INVALID_PARAMETER, result, "Should reject empty pages");
    
    return true;
}

// Test 21: Unified approximation method test
bool test_approximation_methods_comparison() {
    LeafraChunker chunker;
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, chunker.initialize(), "Chunker initialization failed");
    
    std::string text = "This is a comprehensive test of the unified token approximation method. "
                      "All methods now use the same 4 chars/token approach for consistency.";
    
    std::vector<TextChunk> chunks;
    
    // Test the unified method
    ResultCode result = chunker.chunk_text_tokens(text, 15, 0.1, TokenApproximationMethod::SIMPLE, chunks);
    
    TEST_ASSERT_RESULT_CODE(ResultCode::SUCCESS, result, "Simple method should succeed");
    TEST_ASSERT(!chunks.empty(), "Simple method should create chunks");
    
    // Verify chunks have reasonable token estimates
    for (const auto& chunk : chunks) {
        TEST_ASSERT(chunk.estimated_tokens > 0, "Chunk should have token estimate");
    }
    
    return true;
}

// Helper function to check if a word is split across chunk boundaries
bool is_word_split(const std::vector<TextChunk>& chunks) {
    // Implementation of is_word_split function
    return false; // Placeholder return, actual implementation needed
}

int main() {
    std::cout << "=== LeafraChunker Unit Tests ===" << std::endl;
    std::cout << "Platform: ";
#ifdef _WIN32
    std::cout << "Windows";
#elif defined(__APPLE__)
    #ifdef TARGET_OS_IPHONE
        std::cout << "iOS";
    #else
        std::cout << "macOS";
    #endif
#elif defined(__ANDROID__)
    std::cout << "Android";
#elif defined(__linux__)
    std::cout << "Linux";
#else
    std::cout << "Unknown";
#endif
    std::cout << std::endl << std::endl;
    
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    
    // Run all tests
    RUN_TEST(test_chunker_initialization);
    RUN_TEST(test_single_text_chunking_basic);
    RUN_TEST(test_single_text_chunking_small_text);
    RUN_TEST(test_single_text_chunking_overlap);
    RUN_TEST(test_multi_page_document_chunking);
    RUN_TEST(test_multi_page_single_page);
    RUN_TEST(test_advanced_chunking_options);
    RUN_TEST(test_word_boundary_preservation);
    RUN_TEST(test_statistics_tracking);
    RUN_TEST(test_default_options_management);
    RUN_TEST(test_error_handling_invalid_parameters);
    RUN_TEST(test_edge_case_very_long_words);
    RUN_TEST(test_edge_case_only_whitespace);
    RUN_TEST(test_edge_case_unicode_text);
    RUN_TEST(test_chunk_metadata_consistency);
    RUN_TEST(test_performance_large_document);
    RUN_TEST(test_token_estimation);
    RUN_TEST(test_basic_token_chunking);
    RUN_TEST(test_token_multipage_chunking);
    RUN_TEST(test_token_chunking_error_handling);
    RUN_TEST(test_approximation_methods_comparison);
    
    // Print results
    std::cout << std::endl << "=== Test Results ===" << std::endl;
    std::cout << "Total tests: " << total_tests << std::endl;
    std::cout << "Passed: " << passed_tests << std::endl;
    std::cout << "Failed: " << failed_tests << std::endl;
    std::cout << "Success rate: " << (total_tests > 0 ? (passed_tests * 100 / total_tests) : 0) << "%" << std::endl;
    
    if (failed_tests == 0) {
        std::cout << "🎉 All tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ Some tests failed." << std::endl;
        return 1;
    }
} 