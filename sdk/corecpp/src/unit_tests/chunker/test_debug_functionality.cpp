#include "../../../include/leafra/leafra_chunker.h"
#include "../../../include/leafra/leafra_debug.h"
#include "../../../include/leafra/leafra_parsing.h"
#include <iostream>
#include <string>

using namespace leafra;

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
    
    std::cout << "Debug Functionality Test" << std::endl;
    std::cout << "========================" << std::endl;
    
    // Enable debug logging
    std::cout << "Debug enabled: " << (debug_enabled ? "YES" : "NO") << std::endl;
    
    // Test sample text
    std::string test_text = "This is a sample text for testing the chunking functionality with debug output. "
                           "It contains multiple sentences to demonstrate how the chunker works with performance "
                           "measurements and detailed logging. The debug system should show timing information "
                           "and detailed chunk creation steps for analysis and optimization purposes.";
    
    std::cout << "\nTesting with " << test_text.length() << " character sample text" << std::endl;
    
    try {
        // Initialize chunker
        LeafraChunker chunker;
        if (chunker.initialize() != ResultCode::SUCCESS) {
            std::cerr << "Failed to initialize chunker" << std::endl;
            return 1;
        }
        
        // Test different chunk sizes
        std::vector<size_t> test_sizes;
        test_sizes.push_back(50);
        test_sizes.push_back(100);
        test_sizes.push_back(150);
        
        for (size_t chunk_size : test_sizes) {
            std::cout << "\n--- Testing chunk size: " << chunk_size << " tokens ---" << std::endl;
            
            std::vector<TextChunk> chunks;
            ChunkingOptions options(chunk_size, 0.1, ChunkSizeUnit::TOKENS, TokenApproximationMethod::SIMPLE);
            options.preserve_word_boundaries = true;
            
            // This will trigger debug output
            ResultCode result = chunker.chunk_text(test_text, options, chunks);
            
            if (result == ResultCode::SUCCESS) {
                std::cout << "✅ Created " << chunks.size() << " chunks successfully" << std::endl;
                
                // Show summary of chunks
                for (size_t i = 0; i < chunks.size(); ++i) {
                    std::cout << "  Chunk " << (i + 1) << ": " 
                              << chunks[i].estimated_tokens << " tokens, "
                              << chunks[i].content.length() << " chars" << std::endl;
                }
            } else {
                std::cerr << "❌ Chunking failed with result: " << static_cast<int>(result) << std::endl;
            }
        }
        
        // Test with multiple pages (document chunking)
        std::cout << "\n--- Testing document chunking ---" << std::endl;
        std::vector<std::string> pages;
        pages.push_back("First page content: " + test_text.substr(0, test_text.length() / 2));
        pages.push_back("Second page content: " + test_text.substr(test_text.length() / 2));
        
        std::vector<TextChunk> doc_chunks;
        ChunkingOptions doc_options(75, 0.15, ChunkSizeUnit::TOKENS, TokenApproximationMethod::SIMPLE);
        doc_options.preserve_word_boundaries = true;
        
        ResultCode doc_result = chunker.chunk_document(pages, doc_options, doc_chunks);
        
        if (doc_result == ResultCode::SUCCESS) {
            std::cout << "✅ Document chunking created " << doc_chunks.size() << " chunks" << std::endl;
        } else {
            std::cerr << "❌ Document chunking failed" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    // Test file parsing debug functionality
    std::cout << "\n--- Testing file parsing debug ---" << std::endl;
    try {
        FileParsingWrapper parser;
        if (parser.initialize()) {
            std::cout << "✅ File parser initialized" << std::endl;
            
            // Test parsing a small text file (this should be fast)
            std::string test_file = "../../../../../../example/example_files/gift_of_the_magi.txt";
            std::cout << "Parsing: " << test_file << std::endl;
            
            ParsedDocument result = parser.parseFile(test_file);
            
            if (result.isValid) {
                std::cout << "✅ File parsed successfully" << std::endl;
                std::cout << "  Pages: " << result.pages.size() << std::endl;
                if (!result.pages.empty()) {
                    std::cout << "  First page length: " << result.pages[0].length() << " characters" << std::endl;
                }
            } else {
                std::cout << "❌ File parsing failed: " << result.errorMessage << std::endl;
            }
        } else {
            std::cout << "❌ Failed to initialize file parser" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "File parsing exception: " << e.what() << std::endl;
    }
    
    std::cout << "\nDebug functionality test completed!" << std::endl;
    return 0;
} 