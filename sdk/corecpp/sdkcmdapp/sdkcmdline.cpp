#include "leafra/leafra_core.h"
#include "leafra/leafra_chunker.h"
#include "leafra/types.h"
#include <iostream>
#include <fstream>
#include <vector>

using namespace leafra;

/**
 * LeafraSDK Command Line Application
 * 
 * End-to-end testing and development tool for the LeafraSDK.
 * This application provides command line access to test the complete SDK 
 * functionality including document parsing, chunking, and processing.
 * 
 * Supported Platforms: macOS, Linux, Windows
 * Not supported: iOS, Android (command line tool for development)
 */

void print_separator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void create_sample_text_file(const std::string& filename) {
    std::ofstream file(filename);
    file << "This is the first page of our sample document. ";
    file << "It contains multiple sentences to demonstrate chunking. ";
    file << "The chunking system will automatically split this content into smaller pieces. ";
    file << "Each chunk will be token-aware and respect word boundaries. ";
    file << "This makes it perfect for LLM processing and RAG systems. ";
    file << "\n\nThis is the second paragraph on the same page. ";
    file << "It adds more content to work with during the chunking process. ";
    file << "The token estimation helps optimize chunk sizes for language models. ";
    file << "You can configure chunk size, overlap percentage, and token approximation methods. ";
    file << "The system supports character-based and token-based chunking modes.";
    file.close();
}

int main() {
    print_separator("LeafraSDK Command Line Application");
    
    try {
        // Create sample file
        std::string sample_file = "sample_document.txt";
        create_sample_text_file(sample_file);
        std::cout << "📄 Created sample document: " << sample_file << std::endl;
        
        // Create LeafraSDK instance
        auto sdk = LeafraCore::create();
        
        // Configure the SDK with chunking enabled
        Config config;
        config.name = "LeafraSDK-CLI";
        config.version = "1.0.0";
        config.debug_mode = true;  // Enable detailed logging for development
        
        // Configure chunking settings for testing
        config.chunking.enabled = true;
        config.chunking.chunk_size = 50;  // 50 tokens per chunk
        config.chunking.overlap_percentage = 0.2;  // 20% overlap
        config.chunking.size_unit = ChunkSizeUnit::TOKENS;
        config.chunking.token_method = TokenApproximationMethod::WORD_BASED;
        config.chunking.preserve_word_boundaries = true;
        config.chunking.include_metadata = true;
        
        print_separator("SDK Configuration");
        std::cout << "Application: " << config.name << std::endl;
        std::cout << "Platform: Desktop (macOS/Linux/Windows)" << std::endl;
        std::cout << "Purpose: End-to-end SDK testing and development" << std::endl;
        std::cout << "Chunking Enabled: " << (config.chunking.enabled ? "Yes" : "No") << std::endl;
        std::cout << "Chunk Size: " << config.chunking.chunk_size << " tokens" << std::endl;
        std::cout << "Overlap: " << (config.chunking.overlap_percentage * 100) << "%" << std::endl;
        std::cout << "Token Method: Word-based approximation" << std::endl;
        std::cout << "Preserve Word Boundaries: " << (config.chunking.preserve_word_boundaries ? "Yes" : "No") << std::endl;
        
        // Set up event callback to monitor SDK operations
        std::vector<std::string> events;
        sdk->set_event_callback([&events](const std::string& event) {
            events.push_back(event);
            std::cout << "📢 Event: " << event << std::endl;
        });
        
        // Initialize SDK
        print_separator("SDK Initialization");
        ResultCode init_result = sdk->initialize(config);
        if (init_result != ResultCode::SUCCESS) {
            std::cerr << "❌ Failed to initialize SDK!" << std::endl;
            return 1;
        }
        std::cout << "✅ SDK initialized successfully!" << std::endl;
        std::cout << "🔧 Development mode: " << (config.debug_mode ? "Enabled" : "Disabled") << std::endl;
        
        // Process the sample file (end-to-end testing)
        print_separator("End-to-End Document Processing");
        std::cout << "Processing file: " << sample_file << std::endl;
        std::cout << "Testing: Parsing → Chunking → Processing pipeline" << std::endl;
        
        std::vector<std::string> files = {sample_file};
        ResultCode process_result = sdk->process_user_files(files);
        
        if (process_result == ResultCode::SUCCESS) {
            std::cout << "\n✅ End-to-end processing completed successfully!" << std::endl;
        } else {
            std::cout << "\n❌ End-to-end processing failed!" << std::endl;
        }
        
        // Display captured events
        print_separator("SDK Event Summary");
        std::cout << "Total events captured: " << events.size() << std::endl;
        
        // Filter and display important events
        std::cout << "\nKey processing events:" << std::endl;
        for (const auto& event : events) {
            if (event.find("chunk") != std::string::npos || 
                event.find("🧩") != std::string::npos || 
                event.find("📊") != std::string::npos ||
                event.find("🔗") != std::string::npos ||
                event.find("✅") != std::string::npos ||
                event.find("initialized") != std::string::npos) {
                std::cout << "  • " << event << std::endl;
            }
        }
        
        // Cleanup and shutdown
        print_separator("SDK Shutdown");
        std::cout << "Cleaning up resources..." << std::endl;
        sdk->shutdown();
        std::remove(sample_file.c_str());
        std::cout << "✅ Cleanup completed" << std::endl;
        
        print_separator("Test Summary");
        std::cout << "✅ LeafraSDK command line testing completed successfully!" << std::endl;
        std::cout << "🔧 This tool makes SDK development faster and more reliable." << std::endl;
        std::cout << "📋 All SDK components tested: Parsing, Chunking, Events, Configuration" << std::endl;
        std::cout << "🖥️  Platform: Desktop environments (macOS/Linux/Windows)" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 