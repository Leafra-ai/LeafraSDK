#include <iostream>
#include <string>
#include <vector>
#include "../../../include/leafra/leafra_chunker.h"
#include "../../../include/leafra/leafra_debug.h"

using namespace leafra;

void test_unified_api() {
    std::cout << "=== Testing Unified API (No Redundancy) ===" << std::endl;
    
    debug::debug_log("TEST", "Starting unified API tests");
    
    LeafraChunker chunker;
    chunker.initialize();
    
    std::string text = "The quick brown fox jumps over the lazy dog. "
                      "This sentence demonstrates the unified API functionality. "
                      "We can use the same method for both character and token chunking.";
    
    debug::debug_log("TEXT_INFO", "Test text length: " + std::to_string(text.length()) + " characters");
    
    std::vector<TextChunk> chunks;
    
    // Test 1: Character-based chunking using advanced method
    std::cout << "\n1. Character-based chunking (using chunk_text):" << std::endl;
    debug::debug_log("TEST_PHASE", "Starting character-based chunking test");
    
    {
        debug::ScopedTimer timer("Character-based chunking");
        ChunkingOptions char_options(100, 0.1, ChunkSizeUnit::CHARACTERS, TokenApproximationMethod::SIMPLE);
        
        ResultCode result = chunker.chunk_text(text, char_options, chunks);
        if (result == ResultCode::SUCCESS) {
            std::cout << "   ✅ Success! Created " << chunks.size() << " chunks" << std::endl;
            debug::debug_log("SUCCESS", "Character chunking created " + std::to_string(chunks.size()) + " chunks");
            debug::debug_log_performance("Character chunking", text.length(), chunks.size(), timer.elapsed_milliseconds());
            
            for (size_t i = 0; i < chunks.size() && i < 2; ++i) {
                std::cout << "   Chunk " << (i+1) << ": " << chunks[i].content.length() 
                          << " chars, " << chunks[i].estimated_tokens << " tokens" << std::endl;
                debug::debug_log_chunking_details("CHAR_CHUNK", i, chunks[i].start_index, chunks[i].end_index, chunks[i].estimated_tokens, 25);
            }
        } else {
            std::cout << "   ❌ Failed!" << std::endl;
            debug::debug_log("ERROR", "Character-based chunking failed");
        }
    }
    
    // Test 2: Token-based chunking using the SAME advanced method
    std::cout << "\n2. Token-based chunking (using chunk_text):" << std::endl;
    debug::debug_log("TEST_PHASE", "Starting token-based chunking test");
    
    {
        debug::ScopedTimer timer("Token-based chunking");
        ChunkingOptions token_options(20, 0.1, ChunkSizeUnit::TOKENS, TokenApproximationMethod::SIMPLE);
        
        ResultCode result = chunker.chunk_text(text, token_options, chunks);
        if (result == ResultCode::SUCCESS) {
            std::cout << "   ✅ Success! Created " << chunks.size() << " chunks" << std::endl;
            debug::debug_log("SUCCESS", "Token chunking created " + std::to_string(chunks.size()) + " chunks");
            debug::debug_log_performance("Token chunking", text.length(), chunks.size(), timer.elapsed_milliseconds());
            
            for (size_t i = 0; i < chunks.size() && i < 2; ++i) {
                std::cout << "   Chunk " << (i+1) << ": " << chunks[i].content.length() 
                          << " chars, " << chunks[i].estimated_tokens << " tokens" << std::endl;
                debug::debug_log_chunking_details("TOKEN_CHUNK", i, chunks[i].start_index, chunks[i].end_index, chunks[i].estimated_tokens, 20);
            }
        } else {
            std::cout << "   ❌ Failed!" << std::endl;
            debug::debug_log("ERROR", "Token-based chunking failed");
        }
    }
    
    // Test 3: Multi-page document with unified API
    std::cout << "\n3. Multi-page document (using chunk_document):" << std::endl;
    debug::debug_log("TEST_PHASE", "Starting multi-page document test");
    
    std::vector<std::string> pages;
    pages.push_back("Page 1: Introduction with sufficient content for testing purposes.");
    pages.push_back("Page 2: Analysis section containing detailed information and data.");
    
    // Calculate total document size
    size_t total_chars = 0;
    for (const auto& page : pages) {
        total_chars += page.length();
    }
    debug::debug_log("DOC_INFO", "Multi-page document: " + std::to_string(pages.size()) + " pages, " + std::to_string(total_chars) + " total characters");
    
    {
        debug::ScopedTimer timer("Multi-page document chunking");
        ChunkingOptions multipage_options(15, 0.2, ChunkSizeUnit::TOKENS, TokenApproximationMethod::SIMPLE);
        
        ResultCode result = chunker.chunk_document(pages, multipage_options, chunks);
        if (result == ResultCode::SUCCESS) {
            std::cout << "   ✅ Success! Created " << chunks.size() << " chunks across " << pages.size() << " pages" << std::endl;
            debug::debug_log("SUCCESS", "Multi-page chunking created " + std::to_string(chunks.size()) + " chunks from " + std::to_string(pages.size()) + " pages");
            debug::debug_log_performance("Multi-page chunking", total_chars, chunks.size(), timer.elapsed_milliseconds());
            
            for (size_t i = 0; i < chunks.size(); ++i) {
                std::cout << "   Chunk " << (i+1) << " (Page " << chunks[i].page_number 
                          << "): " << chunks[i].estimated_tokens << " tokens" << std::endl;
                debug::debug_log_chunking_details("MULTIPAGE_CHUNK", i, chunks[i].start_index, chunks[i].end_index, chunks[i].estimated_tokens, 15);
            }
        } else {
            std::cout << "   ❌ Failed!" << std::endl;
            debug::debug_log("ERROR", "Multi-page document chunking failed");
        }
    }
    
    std::cout << "\n✨ The unified API eliminates redundancy:" << std::endl;
    std::cout << "   • chunk_text() handles both characters & tokens" << std::endl;
    std::cout << "   • chunk_document() handles both characters & tokens" << std::endl;
    std::cout << "   • Modern API with chunk_text and chunk_document" << std::endl;
    std::cout << "   • Behavior determined by ChunkingOptions.size_unit field" << std::endl;
    
    debug::debug_log("TEST", "Unified API tests completed successfully");
}

// Helper function to setup debug based on command line arguments
bool setup_debug_mode(int argc, char* argv[]) {
    bool debug_enabled = true; // Default: debug enabled
    
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            std::string arg(argv[i]);
            if (arg == "--no-debug" || arg == "--disable-debug") {
                debug_enabled = false;
            } else if (arg == "--debug" || arg == "-d") {
                debug_enabled = true; // Explicitly enable (already default)
            } else if (arg == "--help" || arg == "-h") {
                std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
                std::cout << "Options:" << std::endl;
                std::cout << "  --debug, -d          Enable debug output (default)" << std::endl;
                std::cout << "  --no-debug           Disable debug output" << std::endl;
                std::cout << "  --help, -h           Show this help message" << std::endl;
                exit(0);
            }
        }
    }
    
    debug::set_debug_enabled(debug_enabled);
    return debug_enabled;
}

int main(int argc, char* argv[]) {
    // Setup debug mode based on command line arguments
    bool debug_enabled = setup_debug_mode(argc, argv);
    
    std::cout << "=== Unified API Tests ===" << std::endl;
    std::cout << "Debug mode: " << (debug_enabled ? "ENABLED" : "DISABLED") << std::endl;
    
    try {
        test_unified_api();
        std::cout << "\n🎉 All tests completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 