#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include "../../../include/leafra/leafra_chunker.h"
#include "../../../include/leafra/leafra_debug.h"

using namespace leafra;

void print_separator(const std::string& title) {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << title << std::endl;
    std::cout << std::string(50, '=') << std::endl;
}

void test_token_estimation() {
    print_separator("Token Estimation Tests");
    
    debug::debug_log("TEST", "Starting token estimation tests");
    
    std::string test_text = "This is a sample text with several words for testing token estimation.";
    
    std::cout << "Test text: \"" << test_text << "\"" << std::endl;
    std::cout << "Length: " << test_text.length() << " characters" << std::endl;
    
    // Test all estimation methods with timing
    {
        debug::ScopedTimer timer("Token estimation");
        size_t estimated = LeafraChunker::estimate_token_count(test_text, TokenApproximationMethod::SIMPLE);
        
        std::cout << "\nToken estimates:" << std::endl;
        std::cout << "  Simple (char/4):    " << estimated << " tokens" << std::endl;
        
        debug::debug_log("TOKEN_EST", "Estimated " + std::to_string(estimated) + " tokens for " + std::to_string(test_text.length()) + " characters");
    }
    
    // Test character conversion back
    {
        debug::ScopedTimer timer("Character conversion");
        std::cout << "\nCharacter estimates (for 50 tokens):" << std::endl;
        size_t chars = LeafraChunker::tokens_to_characters(50, TokenApproximationMethod::SIMPLE);
        std::cout << "  Simple:    " << chars << " chars" << std::endl;
        
        debug::debug_log("CHAR_CONV", "50 tokens estimated to " + std::to_string(chars) + " characters");
    }
}

void test_token_chunking() {
    print_separator("Token-Based Chunking Test");
    
    debug::debug_log("TEST", "Starting token-based chunking tests");
    
    std::string sample_text = "The quick brown fox jumps over the lazy dog. "
                             "This is a longer sentence with more words to test chunking functionality. "
                             "We want to see how the token-based chunking works compared to character-based chunking. "
                             "Each chunk should contain approximately the specified number of tokens. "
                             "The accuracy will depend on the approximation method used.";
    
    std::cout << "Sample text length: " << sample_text.length() << " characters" << std::endl;
    debug::debug_log("TEXT_INFO", "Sample text length: " + std::to_string(sample_text.length()) + " characters");
    
    // Test different token sizes and methods
    std::vector<size_t> token_sizes;
    token_sizes.push_back(10);
    token_sizes.push_back(25);
    token_sizes.push_back(50);
    
    std::vector<TokenApproximationMethod> methods;
    methods.push_back(TokenApproximationMethod::SIMPLE);
    
    std::vector<std::string> method_names;
    method_names.push_back("Simple (Unified)");
    
    LeafraChunker chunker;
    if (chunker.initialize() != ResultCode::SUCCESS) {
        std::cout << "Failed to initialize chunker!" << std::endl;
        debug::debug_log("ERROR", "Failed to initialize chunker");
        return;
    }
    
    for (size_t token_size : token_sizes) {
        std::cout << "\n--- Testing " << token_size << " tokens per chunk ---" << std::endl;
        debug::debug_log("TOKEN_SIZE", "Testing " + std::to_string(token_size) + " tokens per chunk");
        
        for (size_t i = 0; i < methods.size(); ++i) {
            debug::ScopedTimer timer("Token chunking " + std::to_string(token_size) + " tokens");
            
            std::vector<TextChunk> chunks;
            ResultCode result = chunker.chunk_text(sample_text, ChunkingOptions(token_size, 0.1, ChunkSizeUnit::TOKENS, methods[i]), chunks);
            
            std::cout << "\n" << method_names[i] << " method:" << std::endl;
            if (result == ResultCode::SUCCESS) {
                std::cout << "  Created " << chunks.size() << " chunks" << std::endl;
                debug::debug_log("CHUNKS", "Created " + std::to_string(chunks.size()) + " chunks for " + std::to_string(token_size) + " token size");
                
                for (size_t j = 0; j < chunks.size() && j < 3; ++j) {  // Show first 3 chunks
                    std::cout << "  Chunk " << (j+1) << " (" << chunks[j].estimated_tokens 
                              << " tokens, " << chunks[j].content.length() << " chars): ";
                    std::string preview = std::string(chunks[j].content.substr(0, 50));
                    if (chunks[j].content.length() > 50) preview += "...";
                    std::cout << "\"" << preview << "\"" << std::endl;
                    
                    debug::debug_log("CHUNK_DETAIL", "Chunk " + std::to_string(j+1) + ": " + 
                                    std::to_string(chunks[j].estimated_tokens) + " tokens, " + 
                                    std::to_string(chunks[j].content.length()) + " chars");
                }
                if (chunks.size() > 3) {
                    std::cout << "  ... (" << (chunks.size() - 3) << " more chunks)" << std::endl;
                }
            } else {
                std::cout << "  ERROR: Failed to chunk text" << std::endl;
                debug::debug_log("ERROR", "Failed to chunk text for " + std::to_string(token_size) + " token size");
            }
        }
    }
}

void test_multi_page_token_chunking() {
    print_separator("Multi-Page Token Chunking Test");
    
    debug::debug_log("TEST", "Starting multi-page token chunking test");
    
    std::vector<std::string> pages;
    pages.push_back("Page 1: Introduction to the document. This page contains basic information and overview.");
    pages.push_back("Page 2: Detailed analysis section. Here we dive deeper into the technical aspects.");
    pages.push_back("Page 3: Conclusions and future work. This section summarizes our findings and recommendations.");
    
    std::cout << "Testing multi-page document with " << pages.size() << " pages" << std::endl;
    debug::debug_log("MULTIPAGE", "Testing document with " + std::to_string(pages.size()) + " pages");
    
    // Calculate total text length
    size_t total_chars = 0;
    for (const auto& page : pages) {
        total_chars += page.length();
    }
    debug::debug_log("TEXT_INFO", "Total document length: " + std::to_string(total_chars) + " characters across " + std::to_string(pages.size()) + " pages");
    
    LeafraChunker chunker;
    if (chunker.initialize() != ResultCode::SUCCESS) {
        std::cout << "Failed to initialize chunker!" << std::endl;
        debug::debug_log("ERROR", "Failed to initialize chunker for multi-page test");
        return;
    }
    
    {
        debug::ScopedTimer timer("Multi-page token chunking");
        
        std::vector<TextChunk> chunks;
        ResultCode result = chunker.chunk_document(pages, ChunkingOptions(15, 0.2, ChunkSizeUnit::TOKENS, TokenApproximationMethod::SIMPLE), chunks);
        
        if (result == ResultCode::SUCCESS) {
            std::cout << "Successfully created " << chunks.size() << " chunks" << std::endl;
            debug::debug_log("SUCCESS", "Created " + std::to_string(chunks.size()) + " chunks from multi-page document");
            
            // Log performance metrics
            debug::debug_log_performance("Multi-page chunking", total_chars, chunks.size(), timer.elapsed_milliseconds());
            
            for (size_t i = 0; i < chunks.size(); ++i) {
                std::cout << "Chunk " << (i+1) << " (Page " << chunks[i].page_number 
                          << ", " << chunks[i].estimated_tokens << " tokens): ";
                std::string preview = std::string(chunks[i].content.substr(0, 60));
                if (chunks[i].content.length() > 60) preview += "...";
                std::cout << "\"" << preview << "\"" << std::endl;
                
                debug::debug_log_chunking_details("MULTIPAGE_CHUNK", i, chunks[i].start_index, chunks[i].end_index, chunks[i].estimated_tokens, 15);
            }
        } else {
            std::cout << "ERROR: Failed to chunk document" << std::endl;
            debug::debug_log("ERROR", "Failed to chunk multi-page document");
        }
    }
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
    
    std::cout << "=== Token-based Chunking Tests ===" << std::endl;
    std::cout << "Debug mode: " << (debug_enabled ? "ENABLED" : "DISABLED") << std::endl;
    
    try {
        test_token_estimation();
        test_token_chunking();
        test_multi_page_token_chunking();
        
        print_separator("All Tests Completed Successfully!");
    } catch (const std::exception& e) {
        std::cout << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 