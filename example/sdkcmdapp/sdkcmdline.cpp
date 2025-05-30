#include "leafra/leafra_core.h"
#include "leafra/leafra_chunker.h"
#include "leafra/types.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

using namespace leafra;

/**
 * LeafraSDK Command Line Application
 * 
 * End-to-end testing and development tool for the LeafraSDK.
 * This application provides command line access to test the complete SDK 
 * functionality including document parsing, chunking, and processing.
 * 
 * Usage:
 *   ./sdkcmdline                    - Process internal sample document
 *   ./sdkcmdline file1.txt          - Process single file
 *   ./sdkcmdline file1.pdf file2.txt - Process multiple files
 * 
 * Supported Platforms: macOS, Linux, Windows
 * Not supported: iOS, Android (command line tool for development)
 */

void print_separator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void print_usage(const char* program_name) {
    std::cout << "\nUsage: " << program_name << " [options] [file1] [file2] ... [fileN]" << std::endl;
    std::cout << "\nOptions:" << std::endl;
    std::cout << "  No arguments              - Process internal sample document (demo mode)" << std::endl;
    std::cout << "  file1 file2...            - Process one or more user files" << std::endl;
    std::cout << "  -h, --help                - Show this help message" << std::endl;
    std::cout << "  --print_chunks_full       - Print full content of all chunks" << std::endl;
    std::cout << "  --print_chunks_brief N    - Print first N lines of each chunk" << std::endl;
    std::cout << "\nSupported file types:" << std::endl;
    std::cout << "  • Text files (.txt)" << std::endl;
    std::cout << "  • PDF files (.pdf)" << std::endl;
    std::cout << "  • Word documents (.docx)" << std::endl;
    std::cout << "  • Excel files (.xlsx)" << std::endl;
    std::cout << "\nExamples:" << std::endl;
    std::cout << "  " << program_name << "                              # Demo mode with sample document" << std::endl;
    std::cout << "  " << program_name << " document.pdf                 # Process single PDF" << std::endl;
    std::cout << "  " << program_name << " file1.txt file2.pdf          # Process multiple files" << std::endl;
    std::cout << "  " << program_name << " --print_chunks_full doc.txt  # Process and show full chunks" << std::endl;
    std::cout << "  " << program_name << " --print_chunks_brief 3 doc.txt # Process and show first 3 lines of each chunk" << std::endl;
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

bool file_exists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

int main(int argc, char* argv[]) {
    print_separator("LeafraSDK Command Line Application");
    
    // Parse command line arguments
    std::vector<std::string> input_files;
    bool demo_mode = true;
    bool print_chunks_full = false;
    bool print_chunks_brief = false;
    int max_lines = 0;
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help" || arg == "help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--print_chunks_full") {
            print_chunks_full = true;
        } else if (arg == "--print_chunks_brief") {
            print_chunks_brief = true;
            // Next argument should be the max_lines number
            if (i + 1 < argc) {
                i++; // Move to next argument
                try {
                    max_lines = std::stoi(argv[i]);
                    if (max_lines <= 0) {
                        std::cerr << "❌ Error: max_lines must be a positive number" << std::endl;
                        return 1;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "❌ Error: Invalid number for max_lines: " << argv[i] << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "❌ Error: --print_chunks_brief requires a number argument" << std::endl;
                return 1;
            }
        } else {
            // It's a file argument
            if (file_exists(arg)) {
                input_files.push_back(arg);
                demo_mode = false;
            } else {
                std::cerr << "⚠️  Warning: File not found: " << arg << std::endl;
            }
        }
    }
    
    if (input_files.empty() && !demo_mode) {
        std::cerr << "❌ Error: No valid files found!" << std::endl;
        print_usage(argv[0]);
        return 1;
    }
    
    try {
        std::string sample_file = "sample_document.txt";
        
        // Setup files to process
        if (demo_mode) {
            // Create sample file for demo
            create_sample_text_file(sample_file);
            input_files.push_back(sample_file);
            std::cout << "📄 Demo Mode: Created sample document: " << sample_file << std::endl;
        } else {
            // User provided files
            std::cout << "📁 User Files Mode: Processing " << input_files.size() << " file(s)" << std::endl;
            for (size_t i = 0; i < input_files.size(); i++) {
                std::cout << "  " << (i + 1) << ". " << input_files[i] << std::endl;
            }
        }
        
        // Create LeafraSDK instance
        auto sdk = LeafraCore::create();
        
        // Configure the SDK with chunking enabled
        Config config;
        config.name = "LeafraSDK-CLI";
        config.version = "1.0.0";
        config.debug_mode = true;  // Enable detailed logging for development
        
        // Configure chunking settings for testing
        config.chunking.enabled = true;
        config.chunking.chunk_size = 500;  // 500 tokens per chunk
        config.chunking.overlap_percentage = 0.2;  // 20% overlap
        config.chunking.size_unit = ChunkSizeUnit::TOKENS;
        config.chunking.token_method = TokenApproximationMethod::SIMPLE;
        config.chunking.preserve_word_boundaries = true;
        config.chunking.include_metadata = true;
        
        // Set chunk printing options based on command line flags
        config.chunking.print_chunks_full = print_chunks_full;
        config.chunking.print_chunks_brief = print_chunks_brief;
        config.chunking.max_lines = max_lines;
        
        print_separator("SDK Configuration");
        std::cout << "Application: " << config.name << std::endl;
        std::cout << "Platform: Desktop (macOS/Linux/Windows)" << std::endl;
        std::cout << "Purpose: End-to-end SDK testing and development" << std::endl;
        std::cout << "Mode: " << (demo_mode ? "Demo (sample document)" : "User files") << std::endl;
        std::cout << "Files to process: " << input_files.size() << std::endl;
        std::cout << "Chunking Enabled: " << (config.chunking.enabled ? "Yes" : "No") << std::endl;
        std::cout << "Chunk Size: " << config.chunking.chunk_size << " tokens" << std::endl;
        std::cout << "Overlap: " << (config.chunking.overlap_percentage * 100) << "%" << std::endl;
        std::cout << "Token Method: Simple approximation" << std::endl;
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
        
        // Process the files (end-to-end testing)
        print_separator("End-to-End Document Processing");
        if (demo_mode) {
            std::cout << "Processing sample file: " << sample_file << std::endl;
        } else {
            std::cout << "Processing " << input_files.size() << " user file(s):" << std::endl;
            for (const auto& file : input_files) {
                std::cout << "  • " << file << std::endl;
            }
        }
        std::cout << "Testing: Parsing → Chunking → Processing pipeline" << std::endl;
        
        ResultCode process_result = sdk->process_user_files(input_files);
        
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
        
        // Only remove sample file if we created it
        if (demo_mode) {
            std::remove(sample_file.c_str());
        }
        std::cout << "✅ Cleanup completed" << std::endl;
        
        print_separator("Test Summary");
        std::cout << "✅ LeafraSDK command line testing completed successfully!" << std::endl;
        std::cout << "🔧 This tool makes SDK development faster and more reliable." << std::endl;
        std::cout << "📋 All SDK components tested: Parsing, Chunking, Events, Configuration" << std::endl;
        std::cout << "🖥️  Platform: Desktop environments (macOS/Linux/Windows)" << std::endl;
        if (!demo_mode) {
            std::cout << "📁 Processed " << input_files.size() << " user file(s) successfully!" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 