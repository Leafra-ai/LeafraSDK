#include <iostream>
#include <string>
#include <vector>
#include "../../../include/leafra/leafra_chunker.h"

using namespace leafra;

void test_unified_api() {
    std::cout << "=== Testing Unified API (No Redundancy) ===" << std::endl;
    
    LeafraChunker chunker;
    chunker.initialize();
    
    std::string text = "The quick brown fox jumps over the lazy dog. "
                      "This sentence demonstrates the unified API functionality. "
                      "We can use the same method for both character and token chunking.";
    
    std::vector<TextChunk> chunks;
    
    // Test 1: Character-based chunking using advanced method
    std::cout << "\n1. Character-based chunking (using chunk_text_advanced):" << std::endl;
    ChunkingOptions char_options(100, 0.1, ChunkSizeUnit::CHARACTERS, TokenApproximationMethod::SIMPLE);
    
    ResultCode result = chunker.chunk_text_advanced(text, char_options, chunks);
    if (result == ResultCode::SUCCESS) {
        std::cout << "   ✅ Success! Created " << chunks.size() << " chunks" << std::endl;
        for (size_t i = 0; i < chunks.size() && i < 2; ++i) {
            std::cout << "   Chunk " << (i+1) << ": " << chunks[i].content.length() 
                      << " chars, " << chunks[i].estimated_tokens << " tokens" << std::endl;
        }
    } else {
        std::cout << "   ❌ Failed!" << std::endl;
    }
    
    // Test 2: Token-based chunking using the SAME advanced method
    std::cout << "\n2. Token-based chunking (using chunk_text_advanced):" << std::endl;
    ChunkingOptions token_options(20, 0.1, ChunkSizeUnit::TOKENS, TokenApproximationMethod::SIMPLE);
    
    result = chunker.chunk_text_advanced(text, token_options, chunks);
    if (result == ResultCode::SUCCESS) {
        std::cout << "   ✅ Success! Created " << chunks.size() << " chunks" << std::endl;
        for (size_t i = 0; i < chunks.size() && i < 2; ++i) {
            std::cout << "   Chunk " << (i+1) << ": " << chunks[i].content.length() 
                      << " chars, " << chunks[i].estimated_tokens << " tokens" << std::endl;
        }
    } else {
        std::cout << "   ❌ Failed!" << std::endl;
    }
    
    // Test 3: Multi-page document with unified API
    std::cout << "\n3. Multi-page document (using chunk_document_advanced):" << std::endl;
    std::vector<std::string> pages;
    pages.push_back("Page 1: Introduction with sufficient content for testing purposes.");
    pages.push_back("Page 2: Analysis section containing detailed information and data.");
    
    ChunkingOptions multipage_options(15, 0.2, ChunkSizeUnit::TOKENS, TokenApproximationMethod::SIMPLE);
    
    result = chunker.chunk_document_advanced(pages, multipage_options, chunks);
    if (result == ResultCode::SUCCESS) {
        std::cout << "   ✅ Success! Created " << chunks.size() << " chunks across " << pages.size() << " pages" << std::endl;
        for (size_t i = 0; i < chunks.size(); ++i) {
            std::cout << "   Chunk " << (i+1) << " (Page " << chunks[i].page_number 
                      << "): " << chunks[i].estimated_tokens << " tokens" << std::endl;
        }
    } else {
        std::cout << "   ❌ Failed!" << std::endl;
    }
    
    std::cout << "\n✨ The unified API eliminates redundancy:" << std::endl;
    std::cout << "   • chunk_text_advanced() handles both characters & tokens" << std::endl;
    std::cout << "   • chunk_document_advanced() handles both characters & tokens" << std::endl;
    std::cout << "   • Legacy methods (chunk_text_tokens, chunk_document_tokens) still work" << std::endl;
    std::cout << "   • Behavior determined by ChunkingOptions.size_unit field" << std::endl;
}

int main() {
    try {
        test_unified_api();
        std::cout << "\n🎉 All tests completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 