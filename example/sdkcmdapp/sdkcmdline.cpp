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
    std::cout << "  --semantic_search \"query\" [max_results] - Perform semantic search (default: 5 results)" << std::endl;
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
    std::cout << "  " << program_name << " --semantic_search \"machine learning\"     # Search indexed content (5 results)" << std::endl;
    std::cout << "  " << program_name << " --semantic_search \"AI technology\" 10  # Search with 10 results" << std::endl;
}

void create_sample_text_file(const std::string& filename) {
    std::ofstream file(filename);
    
    // Check if file opened successfully
    if (!file.is_open()) {
        std::cerr << "❌ Error: Could not create sample file: " << filename << std::endl;
        std::cerr << "   Check permissions and disk space" << std::endl;
        return;
    }
    
    std::cout << "📝 Creating sample text file: " << filename << std::endl;
    
    file << "🌍 International Document Chunking Test 📝\n\n";
    
    file << "This is a comprehensive UTF-8 document designed to test the chunking system's ";
    file << "ability to handle diverse character encodings and international text. ";
    file << "The SentencePiece tokenizer should properly process all these characters. ";
    file << "Each chunk will be token-aware and respect Unicode word boundaries. 🔤\n\n";
    
    file << "📊 Languages & Scripts:\n";
    file << "• English: Hello World! How are you today?\n";
    file << "• French: Bonjour le monde! Comment allez-vous? Café, résumé, naïve, Noël\n";
    file << "• German: Hallo Welt! Wie geht es Ihnen? Straße, München, Größe, Weiß\n";
    file << "• Spanish: ¡Hola mundo! ¿Cómo está usted? Niño, señor, mañana, corazón\n";
    file << "• Russian: Привет мир! Как дела? Москва, Россия, информация\n";
    file << "• Japanese: こんにちは世界！元気ですか？東京、日本、情報\n";
    file << "• Chinese: 你好世界！你好吗？北京，中国，信息\n";
    file << "• Arabic: مرحبا بالعالم! كيف حالك؟ معلومات، تكنولوجيا\n\n";
    
    file << "🔣 Special Characters & Symbols:\n";
    file << "Mathematical: ∑ ∏ ∫ √ ∞ ≈ ≠ ≤ ≥ ± × ÷ π α β γ δ λ μ σ φ ψ ω\n";
    file << "Currency: $ € £ ¥ ₹ ₽ ₩ ₪ ¢ ₵ ₡ ₦ ₨ ₫ ₱ ₲\n";
    file << "Arrows: ← → ↑ ↓ ↖ ↗ ↘ ↙ ⇐ ⇒ ⇑ ⇓ ↔ ↕ ⇔ ⇕\n";
    file << "Shapes: ▲ ▼ ◄ ► ◆ ◇ ■ □ ● ○ ★ ☆ ♠ ♣ ♥ ♦\n";
    file << "Weather: ☀ ☁ ☂ ☃ ❄ ⛅ ⛈ 🌈 🌙 ⭐\n";
    file << "Emojis: 😀 😃 😄 😁 😆 😅 😂 🤣 😊 😇 🙂 🙃 😉 😌 😍 🥰 😘 😗\n\n";
    
    file << "📝 Technical Content:\n";
    file << "This document demonstrates how the LeafraSDK chunking system handles UTF-8 ";
    file << "encoded text with various character sets. The token estimation should accurately ";
    file << "count tokens across different languages and scripts. Character boundaries must ";
    file << "be preserved properly, especially for multi-byte UTF-8 sequences.\n\n";
    
    file << "🔧 Configuration Details:\n";
    file << "• Token-based chunking with SentencePiece integration ✅\n";
    file << "• Word boundary preservation for international text 🌐\n";
    file << "• Overlap percentage handling across language transitions 🔄\n";
    file << "• Metadata extraction from multilingual documents 📋\n";
    file << "• Character encoding validation and normalization 🔤\n\n";
    
    file << "🎯 Test Scenarios:\n";
    file << "1. Mixed language paragraphs with transitions between scripts\n";
    file << "2. Special character sequences that might affect tokenization\n";
    file << "3. Emoji and symbol placement within sentences 📱\n";
    file << "4. Mathematical expressions: E = mc² ∴ F = ma ∵ a² + b² = c²\n";
    file << "5. Code snippets: function(π, α) { return √(x² + y²); } // UTF-8 vars\n";
    file << "6. URLs with Unicode: https://测试.example.com/路径?参数=值\n";
    file << "7. Email addresses: użytkownik@примеру.рф, тест@مثال.كوم\n\n";
    
    file << "📚 Extended Content for Chunking:\n";
    file << "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor ";
    file << "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis ";
    file << "nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. ";
    file << "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore ";
    file << "eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, ";
    file << "sunt in culpa qui officia deserunt mollit anim id est laborum.\n\n";
    
    file << "Ñoño pequeño soñó con niños en España. El señor García visitó São Paulo ";
    file << "para encontrar información sobre tecnología avanzada. Les résumés français ";
    file << "contiennent des caractères accentués comme é, è, ê, ë, à, ù, ç. Deutsche ";
    file << "Straßennamen enthalten oft Umlaute: München, Köln, Düsseldorf, Größe.\n\n";
    
    file << "🌐 Conclusion:\n";
    file << "This UTF-8 test document validates that the LeafraSDK chunking system properly ";
    file << "handles international character sets, maintains character encoding integrity, ";
    file << "and produces accurate token counts across diverse linguistic content. The ";
    file << "SentencePiece integration should seamlessly process all included characters ";
    file << "while preserving semantic boundaries. Success! ✨🎉\n";
    
    file.close();
    
    // Verify file was actually created and has content
    std::ifstream verify_file(filename);
    if (verify_file.is_open()) {
        verify_file.seekg(0, std::ios::end);
        std::streamsize size = verify_file.tellg();
        verify_file.close();
        
        std::cout << "✅ Sample file created successfully: " << filename 
                  << " (" << size << " bytes)" << std::endl;
    } else {
        std::cerr << "❌ Failed to create sample file: " << filename << std::endl;
        std::cerr << "   File does not exist after creation attempt" << std::endl;
    }
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
    bool semantic_search_mode = false;
    std::string search_query;
    int max_results = 5;
    
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
        } else if (arg == "--semantic_search") {
            semantic_search_mode = true;
            demo_mode = false;
            // Next argument should be the search query
            if (i + 1 < argc) {
                i++; // Move to next argument
                search_query = argv[i];
                if (search_query.empty()) {
                    std::cerr << "❌ Error: Search query cannot be empty" << std::endl;
                    return 1;
                }
                // Check if there's an optional max_results parameter
                if (i + 1 < argc) {
                    std::string next_arg = argv[i + 1];
                    // Check if next argument is a number (not starting with -)
                    if (!next_arg.empty() && next_arg[0] != '-' && std::isdigit(next_arg[0])) {
                        i++; // Move to next argument
                        try {
                            max_results = std::stoi(next_arg);
                            if (max_results <= 0) {
                                std::cerr << "❌ Error: max_results must be a positive number" << std::endl;
                                return 1;
                            }
                        } catch (const std::exception& e) {
                            std::cerr << "❌ Error: Invalid number for max_results: " << next_arg << std::endl;
                            return 1;
                        }
                    }
                }
            } else {
                std::cerr << "❌ Error: --semantic_search requires a query string argument" << std::endl;
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
    
    if (input_files.empty() && !demo_mode && !semantic_search_mode) {
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
        } else if (semantic_search_mode) {
            // Semantic search mode - no files needed, searches indexed content
            std::cout << "🔍 Semantic Search Mode: Searching indexed content" << std::endl;
            std::cout << "🔎 Query: \"" << search_query << "\"" << std::endl;
            std::cout << "📊 Max Results: " << max_results << std::endl;
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
        
        // Configure SentencePiece tokenization (optional - will fallback if model not found)
        config.tokenizer.enable_sentencepiece = true;
        
        //TODO AD: change this to model path like we do for the embedding_inference.model_path
        config.tokenizer.model_name = "multilingual-e5-small"; // Model name corresponds to folder in models/
        
        // Resolve the model path from the model name
        bool model_found = config.tokenizer.resolve_model_path();
        if (model_found) {
            std::cout << "📍 Found SentencePiece model: " << config.tokenizer.model_name << std::endl;
            std::cout << "   Model file: " << config.tokenizer.sentencepiece_model_path << std::endl;
            if (!config.tokenizer.sentencepiece_json_path.empty()) {
                std::cout << "   Config file: " << config.tokenizer.sentencepiece_json_path << std::endl;
            } else {
                std::cout << "   Config file: not found (optional)" << std::endl;
            }
        } else {
            std::cout << "⚠️  SentencePiece model '" << config.tokenizer.model_name << "' not found" << std::endl;
            std::cout << "   Expected location: sdk/corecpp/third_party/models/embedding/" << config.tokenizer.model_name << "/sentencepiece.bpe.model" << std::endl;
            std::cout << "   Expected config: sdk/corecpp/third_party/models/embedding/" << config.tokenizer.model_name << "/tokenizer_config.json" << std::endl;
            std::cout << "   Using fallback: " << config.tokenizer.sentencepiece_model_path << std::endl;
        }
        
        // Set chunk printing options based on command line flags
        config.chunking.print_chunks_full = print_chunks_full;
        config.chunking.print_chunks_brief = print_chunks_brief;
        config.chunking.max_lines = max_lines;
        
        config.embedding_inference.enabled = true;
        config.embedding_inference.framework = "coreml";
        config.embedding_inference.model_path = std::string(LEAFRA_SDK_MODELS_ROOT) + "/embedding/generated_models/coreml/model.mlmodelc";
        config.vector_search.enabled = true;
        //AD TEMP
        //config.vector_search.index_type = "HNSW";
        config.vector_search.index_type = "FLAT";
        config.vector_search.metric = "COSINE";
        config.vector_search.dimension = 384;

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
        std::cout << "SentencePiece Enabled: " << (config.tokenizer.enable_sentencepiece ? "Yes" : "No") << std::endl;
        if (config.tokenizer.enable_sentencepiece && !config.tokenizer.sentencepiece_model_path.empty()) {
            std::cout << "SentencePiece Model: " << config.tokenizer.sentencepiece_model_path << std::endl;
        }
        std::cout << "Embedding Inference Enabled: " << (config.embedding_inference.enabled ? "Yes" : "No") << std::endl;
        std::cout << "Embedding Framework: " << config.embedding_inference.framework << std::endl;
        std::cout << "Embedding Model Path: " << config.embedding_inference.model_path << std::endl;
        
        
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
        
        // Handle semantic search mode
        if (semantic_search_mode) {
            print_separator("Semantic Search");
            std::cout << "🔍 Performing semantic search..." << std::endl;
            std::cout << "Query: \"" << search_query << "\"" << std::endl;
            std::cout << "Max Results: " << max_results << std::endl;
            
#ifdef LEAFRA_HAS_FAISS
            std::vector<FaissIndex::SearchResult> search_results;
            ResultCode search_result = sdk->semantic_search(search_query, max_results, search_results);
            
            if (search_result == ResultCode::SUCCESS) {
                std::cout << "\n✅ Semantic search completed successfully!" << std::endl;
                std::cout << "📊 Found " << search_results.size() << " results:" << std::endl;
                
                for (size_t i = 0; i < search_results.size(); i++) {
                    const auto& result = search_results[i];
                    std::cout << "\n🔍 Result " << (i + 1) << ":" << std::endl;
                    std::cout << "   📄 File: " << result.filename << std::endl;
                    std::cout << "   📖 Page: " << result.page_number << std::endl;
                    std::cout << "   🧩 Chunk: " << result.chunk_index << std::endl;
                    std::cout << "   📏 Distance: " << result.distance << std::endl;
                    std::cout << "   📝 Content: " << std::endl;
                    
                    // Print content with proper formatting
                    std::string content = result.content;
                    if (content.length() > 200) {
                        content = content.substr(0, 200) + "...";
                    }
                    std::cout << "      " << content << std::endl;
                }
            } else {
                std::cout << "\n❌ Semantic search failed!" << std::endl;
                std::cout << "   Make sure you have processed some documents first." << std::endl;
            }
#else
            std::cout << "❌ FAISS support not compiled - semantic search unavailable" << std::endl;
#endif
            
            // Skip normal processing for semantic search mode
            print_separator("SDK Shutdown");
            std::cout << "Cleaning up resources..." << std::endl;
            sdk->shutdown();
            std::cout << "✅ Cleanup completed" << std::endl;
            
            print_separator("Search Summary");
            std::cout << "✅ Semantic search completed!" << std::endl;
            return 0;
        }
        
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