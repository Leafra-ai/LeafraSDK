#include "leafra/leafra_core.h"
#include "leafra/data_processor.h"
#include "leafra/math_utils.h"
#include "leafra/platform_utils.h"
#include "leafra/logger.h"
#include "leafra/leafra_parsing.h"
#include "leafra/leafra_chunker.h"
#include <iostream>
#include <sstream>

#ifdef LEAFRA_HAS_PDFIUM
#include "fpdfview.h"
#include "fpdf_text.h"
#include "fpdf_doc.h"
#endif

namespace leafra {

// Implementation of ChunkingConfig constructors
ChunkingConfig::ChunkingConfig() {
    // Default values are already set in the header with sensible defaults for LLM processing
    size_unit = ChunkSizeUnit::TOKENS;
    token_method = TokenApproximationMethod::SIMPLE;
}

ChunkingConfig::ChunkingConfig(size_t size, double overlap, bool use_tokens) 
    : chunk_size(size), overlap_percentage(overlap) {
    size_unit = use_tokens ? ChunkSizeUnit::TOKENS : ChunkSizeUnit::CHARACTERS;
    token_method = TokenApproximationMethod::SIMPLE;
}

// Private implementation class (PIMPL pattern)
class LeafraCore::Impl {
public:
    Config config_;
    bool initialized_;
    callback_t event_callback_;
    std::unique_ptr<DataProcessor> data_processor_;
    std::unique_ptr<MathUtils> math_utils_;
    std::unique_ptr<FileParsingWrapper> file_parser_;
    std::unique_ptr<LeafraChunker> chunker_;
    
    Impl() : initialized_(false), event_callback_(nullptr) {
        data_processor_ = std::make_unique<DataProcessor>();
        math_utils_ = std::make_unique<MathUtils>();
        file_parser_ = std::make_unique<FileParsingWrapper>();
        chunker_ = std::make_unique<LeafraChunker>();
    }
    
    void send_event(const std::string& message) {
        if (event_callback_) {
            event_callback_(message.c_str());
        }
    }
};

LeafraCore::LeafraCore() : pImpl(std::make_unique<Impl>()) {
}

LeafraCore::~LeafraCore() = default;

ResultCode LeafraCore::initialize(const Config& config) {
    if (pImpl->initialized_) {
        return ResultCode::SUCCESS; // Already initialized
    }
    
    try {
        pImpl->config_ = config;
        
        // Initialize logging system
        Logger& logger = Logger::getInstance();
        if (config.debug_mode) {
            logger.setLogLevel(LogLevel::LEAFRA_DEBUG);
            LEAFRA_INFO() << "Debug logging enabled";
        } else {
            logger.setLogLevel(LogLevel::LEAFRA_INFO);
        }
        
        LEAFRA_INFO() << "Initializing LeafraSDK v" << get_version();
        LEAFRA_DEBUG() << "Config - Name: " << config.name << ", Threads: " << config.max_threads << ", Buffer: " << config.buffer_size;
        
        // Initialize data processor
        if (pImpl->data_processor_) {
            ResultCode result = pImpl->data_processor_->initialize();
            if (result != ResultCode::SUCCESS) {
                LEAFRA_ERROR() << "Failed to initialize data processor";
                return result;
            }
            LEAFRA_DEBUG() << "Data processor initialized successfully";
        }
        
        // Initialize file parser
        if (pImpl->file_parser_) {
            bool result = pImpl->file_parser_->initialize();
            if (!result) {
                LEAFRA_ERROR() << "Failed to initialize file parser";
                return ResultCode::ERROR_INITIALIZATION_FAILED;
            }
            LEAFRA_DEBUG() << "File parser initialized successfully";
        }
        
        // Initialize chunker
        if (pImpl->chunker_) {
            ResultCode result = pImpl->chunker_->initialize();
            if (result != ResultCode::SUCCESS) {
                LEAFRA_ERROR() << "Failed to initialize chunker";
                return result;
            }
            LEAFRA_DEBUG() << "Chunker initialized successfully";
            
            // Configure chunker with config settings
            ChunkingOptions chunking_options(
                config.chunking.chunk_size,
                config.chunking.overlap_percentage,
                config.chunking.size_unit,
                config.chunking.token_method
            );
            chunking_options.preserve_word_boundaries = config.chunking.preserve_word_boundaries;
            chunking_options.include_metadata = config.chunking.include_metadata;
            
            pImpl->chunker_->set_default_options(chunking_options);
            
            // Log chunking configuration
            LEAFRA_INFO() << "Chunking configuration:";
            LEAFRA_INFO() << "  - Enabled: " << (config.chunking.enabled ? "Yes" : "No");
            LEAFRA_INFO() << "  - Chunk size: " << config.chunking.chunk_size 
                          << (config.chunking.size_unit == ChunkSizeUnit::TOKENS ? " tokens" : " characters");
            LEAFRA_INFO() << "  - Overlap: " << (config.chunking.overlap_percentage * 100.0) << "%";
            LEAFRA_INFO() << "  - Token method: Simple";
        }
        
        pImpl->initialized_ = true;
        LEAFRA_INFO() << "LeafraSDK initialized successfully";
        
#ifdef LEAFRA_HAS_PDFIUM
        LEAFRA_INFO() << "✅ PDFium integration: ENABLED";
#else
        LEAFRA_WARNING() << "⚠️  PDFium integration: DISABLED (library not found)";
#endif
        
        pImpl->send_event("LeafraSDK initialized successfully");
        
        return ResultCode::SUCCESS;
    } catch (const std::exception& e) {
        LEAFRA_ERROR() << "Initialization failed: " << e.what();
        pImpl->send_event("Initialization failed: " + std::string(e.what()));
        return ResultCode::ERROR_INITIALIZATION_FAILED;
    }
}

ResultCode LeafraCore::shutdown() {
    if (!pImpl->initialized_) {
        return ResultCode::SUCCESS; // Already shutdown
    }
    
    try {
        // Shutdown chunker
        if (pImpl->chunker_) {
            // Note: If LeafraChunker doesn't have a shutdown method, the destructor will handle cleanup
            // But we reset statistics and clear any cached data
            pImpl->chunker_->reset_statistics();
            LEAFRA_DEBUG() << "Chunker shutdown completed";
        }
        
        // Shutdown file parser
        if (pImpl->file_parser_) {
            pImpl->file_parser_->shutdown();
            LEAFRA_DEBUG() << "File parser shutdown completed";
        }
        
        pImpl->initialized_ = false;
        pImpl->send_event("LeafraSDK shutdown completed");
        return ResultCode::SUCCESS;
    } catch (const std::exception& e) {
        pImpl->send_event("Shutdown failed: " + std::string(e.what()));
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

bool LeafraCore::is_initialized() const {
    return pImpl->initialized_;
}

const Config& LeafraCore::get_config() const {
    return pImpl->config_;
}

ResultCode LeafraCore::process_data(const data_buffer_t& input, data_buffer_t& output) {
    if (!pImpl->initialized_) {
        return ResultCode::ERROR_INITIALIZATION_FAILED;
    }
    
    if (!pImpl->data_processor_) {
        return ResultCode::ERROR_NOT_IMPLEMENTED;
    }
    
    try {
        return pImpl->data_processor_->process(input, output);
    } catch (const std::exception& e) {
        pImpl->send_event("Data processing failed: " + std::string(e.what()));
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

ResultCode LeafraCore::process_user_files(const std::vector<std::string>& file_paths) {
    if (!pImpl->initialized_) {
        LEAFRA_ERROR() << "LeafraCore not initialized";
        return ResultCode::ERROR_INITIALIZATION_FAILED;
    }
    
    if (!pImpl->file_parser_) {
        LEAFRA_ERROR() << "File parser not available";
        return ResultCode::ERROR_NOT_IMPLEMENTED;
    }
    
    LEAFRA_INFO() << "Processing " << file_paths.size() << " user files";
    pImpl->send_event("Processing " + std::to_string(file_paths.size()) + " user files");
    
    size_t processed_count = 0;
    size_t error_count = 0;
    
    for (const auto& file_path : file_paths) {
        LEAFRA_INFO() << "Processing file: " << file_path;
        pImpl->send_event("Processing file: " + file_path);
        
        // Check if file type is supported
        if (!pImpl->file_parser_->isFileTypeSupported(file_path)) {
            LEAFRA_WARNING() << "Unsupported file type: " << file_path;
            pImpl->send_event("Unsupported file type: " + file_path);
            error_count++;
            continue;
        }
        
        // Parse the file using the appropriate adapter
        ParsedDocument result = pImpl->file_parser_->parseFile(file_path);
        
        if (result.isValid) {
            processed_count++;
            
            // Log parsing results
            LEAFRA_INFO() << "Successfully parsed " << result.fileType << " file: " << file_path;
            LEAFRA_INFO() << "  - Title: " << result.title;
            LEAFRA_INFO() << "  - Author: " << result.author;
            LEAFRA_INFO() << "  - Pages: " << result.getPageCount();
            LEAFRA_INFO() << "  - Total text length: " << result.getAllText().length() << " characters";
            
            // Send detailed events
            pImpl->send_event("✅ Parsed " + result.fileType + ": " + file_path);
            pImpl->send_event("📄 Pages: " + std::to_string(result.getPageCount()));
            pImpl->send_event("📝 Text length: " + std::to_string(result.getAllText().length()) + " chars");
            
            if (!result.title.empty()) {
                pImpl->send_event("📖 Title: " + result.title);
            }
            if (!result.author.empty()) {
                pImpl->send_event("👤 Author: " + result.author);
            }
            
            // Log metadata if available
            for (const auto& [key, value] : result.metadata) {
                if (!value.empty()) {
                    LEAFRA_DEBUG() << "  - " << key << ": " << value;
                }
            }
            
            // Perform chunking if enabled
            if (pImpl->config_.chunking.enabled && pImpl->chunker_) {
                LEAFRA_INFO() << "Starting chunking process for: " << file_path;
                pImpl->send_event("🔗 Starting chunking process");
                
                // Prepare pages for chunking
                std::vector<std::string> pages;
                for (size_t i = 0; i < result.getPageCount(); ++i) {
                    if (i < result.pages.size() && !result.pages[i].empty()) {
                        pages.push_back(result.pages[i]);
                    }
                }
                
                if (!pages.empty()) {
                    std::vector<TextChunk> chunks;
                    ResultCode chunk_result = pImpl->chunker_->chunk_document_advanced(pages, pImpl->chunker_->get_default_options(), chunks);
                    
                    if (chunk_result == ResultCode::SUCCESS) {
                        LEAFRA_INFO() << "✅ Successfully created " << chunks.size() << " chunks";
                        pImpl->send_event("🧩 Created " + std::to_string(chunks.size()) + " chunks");
                        
                        // Log chunk statistics
                        size_t total_chunk_chars = 0;
                        size_t total_estimated_tokens = 0;
                        for (const auto& chunk : chunks) {
                            total_chunk_chars += chunk.content.length();
                            total_estimated_tokens += chunk.estimated_tokens;
                        }
                        
                        LEAFRA_INFO() << "Chunk statistics:";
                        LEAFRA_INFO() << "  - Total chunks: " << chunks.size();
                        LEAFRA_INFO() << "  - Total characters in chunks: " << total_chunk_chars;
                        LEAFRA_INFO() << "  - Estimated tokens: " << total_estimated_tokens;
                        LEAFRA_INFO() << "  - Avg chunk size: " << (chunks.size() > 0 ? total_chunk_chars / chunks.size() : 0) << " chars";
                        LEAFRA_INFO() << "  - Avg tokens per chunk: " << (chunks.size() > 0 ? total_estimated_tokens / chunks.size() : 0);
                        
                        pImpl->send_event("📊 Chunks: " + std::to_string(chunks.size()) + 
                                        ", Avg size: " + std::to_string(chunks.size() > 0 ? total_chunk_chars / chunks.size() : 0) + " chars, " +
                                        std::to_string(chunks.size() > 0 ? total_estimated_tokens / chunks.size() : 0) + " tokens");
                        
                        // Print detailed chunk content if requested (development/debug feature)
                        if (pImpl->config_.chunking.print_chunks_full || pImpl->config_.chunking.print_chunks_brief) {
                            LEAFRA_INFO() << "";
                            LEAFRA_INFO() << "============================================================";
                            LEAFRA_INFO() << "  Chunk Content Analysis for: " << file_path;
                            LEAFRA_INFO() << "============================================================";
                            LEAFRA_INFO() << "📋 Chunk printing requested - showing chunker output:";
                            LEAFRA_INFO() << "📊 Created " << chunks.size() << " chunks from " << file_path;
                            
                            for (size_t i = 0; i < chunks.size(); ++i) {
                                const auto& chunk = chunks[i];
                                
                                LEAFRA_INFO() << "";
                                LEAFRA_INFO() << "----------------------------------------";
                                LEAFRA_INFO() << "Chunk " << (i + 1) << " of " << chunks.size() << ":";
                                LEAFRA_INFO() << "  📐 Length: " << chunk.content.length() << " characters";
                                LEAFRA_INFO() << "  🔤 Tokens: " << chunk.estimated_tokens << " (estimated)";
                                LEAFRA_INFO() << "  📄 Page: " << (chunk.page_number + 1);
                                LEAFRA_INFO() << "  📍 Position: " << chunk.start_index << "-" << chunk.end_index;
                                LEAFRA_INFO() << "Content:";
                                
                                // Print content based on print mode
                                if (pImpl->config_.chunking.print_chunks_full) {
                                    // Print full content
                                    LEAFRA_INFO() << chunk.content;
                                } else if (pImpl->config_.chunking.print_chunks_brief) {
                                    // Print first N lines
                                    std::istringstream stream(chunk.content);
                                    std::string line;
                                    int line_count = 0;
                                    int max_lines = pImpl->config_.chunking.max_lines;
                                    
                                    while (std::getline(stream, line) && line_count < max_lines) {
                                        LEAFRA_INFO() << line;
                                        line_count++;
                                    }
                                    
                                    // Check if there are more lines
                                    if (std::getline(stream, line)) {
                                        LEAFRA_INFO() << "... (content truncated, " << max_lines << " lines shown)";
                                    }
                                }
                                
                                if (i < chunks.size() - 1) {
                                    LEAFRA_INFO() << "";
                                }
                            }
                            
                            LEAFRA_INFO() << "============================================================";
                        }
                        
                        // Optional: Log first few chunks for debugging (only in debug mode)
                        if (pImpl->config_.debug_mode && chunks.size() > 0) {
                            size_t chunks_to_log = std::min(size_t(3), chunks.size());
                            for (size_t i = 0; i < chunks_to_log; ++i) {
                                LEAFRA_DEBUG() << "Chunk " << (i + 1) << " (page " << chunks[i].page_number + 1 
                                             << ", " << chunks[i].content.length() << " chars, " 
                                             << chunks[i].estimated_tokens << " tokens): "
                                             << chunks[i].content.substr(0, 100) << "...";
                            }
                        }
                        
                    } else {
                        LEAFRA_ERROR() << "Failed to chunk document: " << file_path;
                        pImpl->send_event("❌ Chunking failed for: " + file_path);
                    }
                } else {
                    LEAFRA_WARNING() << "No text content found for chunking in: " << file_path;
                    pImpl->send_event("⚠️ No text content for chunking");
                }
            } else if (!pImpl->config_.chunking.enabled) {
                LEAFRA_DEBUG() << "Chunking disabled in configuration, skipping chunk creation";
            }
            
        } else {
            error_count++;
            LEAFRA_ERROR() << "Failed to parse file: " << file_path << " - " << result.errorMessage;
            pImpl->send_event("❌ Failed to parse: " + file_path + " - " + result.errorMessage);
        }
    }
    
    // Summary
    LEAFRA_INFO() << "File processing completed - Processed: " << processed_count 
                  << ", Errors: " << error_count << ", Total: " << file_paths.size();
    
    pImpl->send_event("📊 Processing summary: " + std::to_string(processed_count) + 
                     " successful, " + std::to_string(error_count) + " failed");
    
    if (processed_count > 0) {
        pImpl->send_event("✅ File processing completed successfully");
        return ResultCode::SUCCESS;
    } else if (error_count == file_paths.size()) {
        pImpl->send_event("❌ All files failed to process");
        return ResultCode::ERROR_PROCESSING_FAILED;
    } else {
        pImpl->send_event("⚠️ File processing completed with some errors");
        return ResultCode::SUCCESS; // Partial success
    }
}

void LeafraCore::set_event_callback(callback_t callback) {
    pImpl->event_callback_ = callback;
}

std::string LeafraCore::get_version() {
    std::ostringstream oss;
    oss << LEAFRA_VERSION_MAJOR << "." << LEAFRA_VERSION_MINOR << "." << LEAFRA_VERSION_PATCH;
    return oss.str();
}

std::string LeafraCore::get_platform() {
    return PlatformUtils::get_platform_name();
}

shared_ptr<LeafraCore> LeafraCore::create() {
    return std::make_shared<LeafraCore>();
}

} // namespace leafra 