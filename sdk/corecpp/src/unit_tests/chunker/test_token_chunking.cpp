#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include "../../../include/leafra/leafra_chunker.h"

using namespace leafra;

void print_separator(const std::string& title) {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << title << std::endl;
    std::cout << std::string(50, '=') << std::endl;
}

void test_token_estimation() {
    print_separator("Token Estimation Tests");
    
    std::string test_text = "This is a sample text with several words for testing token estimation.";
    
    std::cout << "Test text: \"" << test_text << "\"" << std::endl;
    std::cout << "Length: " << test_text.length() << " characters" << std::endl;
    
    // Test all estimation methods
    size_t simple = LeafraChunker::estimate_token_count(test_text, TokenApproximationMethod::SIMPLE);
    size_t word_based = LeafraChunker::estimate_token_count(test_text, TokenApproximationMethod::WORD_BASED);
    size_t advanced = LeafraChunker::estimate_token_count(test_text, TokenApproximationMethod::ADVANCED);
    
    std::cout << "\nToken estimates:" << std::endl;
    std::cout << "  Simple (char/4):    " << simple << " tokens" << std::endl;
    std::cout << "  Word-based (×1.33): " << word_based << " tokens" << std::endl;
    std::cout << "  Advanced heuristic: " << advanced << " tokens" << std::endl;
    
    // Test character conversion back
    std::cout << "\nCharacter estimates (for 50 tokens):" << std::endl;
    std::cout << "  Simple:    " << LeafraChunker::tokens_to_characters(50, TokenApproximationMethod::SIMPLE) << " chars" << std::endl;
    std::cout << "  Word-based: " << LeafraChunker::tokens_to_characters(50, TokenApproximationMethod::WORD_BASED) << " chars" << std::endl;
    std::cout << "  Advanced:  " << LeafraChunker::tokens_to_characters(50, TokenApproximationMethod::ADVANCED) << " chars" << std::endl;
}

void test_token_chunking() {
    print_separator("Token-Based Chunking Test");
    
    std::string sample_text = "The quick brown fox jumps over the lazy dog. "
                             "This is a longer sentence with more words to test chunking functionality. "
                             "We want to see how the token-based chunking works compared to character-based chunking. "
                             "Each chunk should contain approximately the specified number of tokens. "
                             "The accuracy will depend on the approximation method used.";
    
    std::cout << "Sample text length: " << sample_text.length() << " characters" << std::endl;
    
    // Test different token sizes and methods
    std::vector<size_t> token_sizes;
    token_sizes.push_back(10);
    token_sizes.push_back(25);
    token_sizes.push_back(50);
    
    std::vector<TokenApproximationMethod> methods;
    methods.push_back(TokenApproximationMethod::SIMPLE);
    methods.push_back(TokenApproximationMethod::WORD_BASED);
    methods.push_back(TokenApproximationMethod::ADVANCED);
    
    std::vector<std::string> method_names;
    method_names.push_back("Simple");
    method_names.push_back("Word-based");
    method_names.push_back("Advanced");
    
    LeafraChunker chunker;
    if (chunker.initialize() != ResultCode::SUCCESS) {
        std::cout << "Failed to initialize chunker!" << std::endl;
        return;
    }
    
    for (size_t token_size : token_sizes) {
        std::cout << "\n--- Testing " << token_size << " tokens per chunk ---" << std::endl;
        
        for (size_t i = 0; i < methods.size(); ++i) {
            std::vector<TextChunk> chunks;
            ResultCode result = chunker.chunk_text_tokens(sample_text, token_size, 0.1, methods[i], chunks);
            
            std::cout << "\n" << method_names[i] << " method:" << std::endl;
            if (result == ResultCode::SUCCESS) {
                std::cout << "  Created " << chunks.size() << " chunks" << std::endl;
                
                for (size_t j = 0; j < chunks.size() && j < 3; ++j) {  // Show first 3 chunks
                    std::cout << "  Chunk " << (j+1) << " (" << chunks[j].estimated_tokens 
                              << " tokens, " << chunks[j].content.length() << " chars): ";
                    std::string preview = chunks[j].content.substr(0, 50);
                    if (chunks[j].content.length() > 50) preview += "...";
                    std::cout << "\"" << preview << "\"" << std::endl;
                }
                if (chunks.size() > 3) {
                    std::cout << "  ... (" << (chunks.size() - 3) << " more chunks)" << std::endl;
                }
            } else {
                std::cout << "  ERROR: Failed to chunk text" << std::endl;
            }
        }
    }
}

void test_multi_page_token_chunking() {
    print_separator("Multi-Page Token Chunking Test");
    
    std::vector<std::string> pages;
    pages.push_back("Page 1: Introduction to the document. This page contains basic information and overview.");
    pages.push_back("Page 2: Detailed analysis section. Here we dive deeper into the technical aspects.");
    pages.push_back("Page 3: Conclusions and future work. This section summarizes our findings and recommendations.");
    
    std::cout << "Testing multi-page document with " << pages.size() << " pages" << std::endl;
    
    LeafraChunker chunker;
    if (chunker.initialize() != ResultCode::SUCCESS) {
        std::cout << "Failed to initialize chunker!" << std::endl;
        return;
    }
    
    std::vector<TextChunk> chunks;
    ResultCode result = chunker.chunk_document_tokens(pages, 15, 0.2, TokenApproximationMethod::WORD_BASED, chunks);
    
    if (result == ResultCode::SUCCESS) {
        std::cout << "Successfully created " << chunks.size() << " chunks" << std::endl;
        
        for (size_t i = 0; i < chunks.size(); ++i) {
            std::cout << "Chunk " << (i+1) << " (Page " << chunks[i].page_number 
                      << ", " << chunks[i].estimated_tokens << " tokens): ";
            std::string preview = chunks[i].content.substr(0, 60);
            if (chunks[i].content.length() > 60) preview += "...";
            std::cout << "\"" << preview << "\"" << std::endl;
        }
    } else {
        std::cout << "ERROR: Failed to chunk document" << std::endl;
    }
}

int main() {
    std::cout << "Testing LeafraChunker Token-Based Functionality" << std::endl;
    std::cout << "=============================================" << std::endl;
    
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