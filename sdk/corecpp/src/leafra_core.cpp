#include "leafra/leafra_core.h"
#include "leafra/data_processor.h"
#include "leafra/math_utils.h"
#include "leafra/platform_utils.h"
#include "leafra/logger.h"
#include "leafra/leafra_parsing.h"
#include "leafra/leafra_chunker.h"
#include "leafra/leafra_sentencepiece.h"
#include <iostream>
#include <sstream>
#include <fstream>

#ifdef LEAFRA_HAS_PDFIUM
#include "fpdfview.h"
#include "fpdf_text.h"
#include "fpdf_doc.h"
#endif

#ifdef LEAFRA_HAS_TENSORFLOWLITE
// TensorFlow Lite C API headers - start with core functionality only
#include "c_api.h"
#include "c_api_types.h"
#include "builtin_ops.h"

// Note: Starting with core TensorFlow Lite functionality only
// Delegates will be added in a future update
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
    std::unique_ptr<SentencePieceTokenizer> tokenizer_;
    
#ifdef LEAFRA_HAS_TENSORFLOWLITE
    // TensorFlow Lite inference components
    TfLiteModel* tf_model_;
    TfLiteInterpreter* tf_interpreter_;
    TfLiteInterpreterOptions* tf_options_;
    std::vector<TfLiteDelegate*> tf_delegates_;
    bool tf_initialized_;
#endif
    
    Impl() : initialized_(false), event_callback_(nullptr) {
        data_processor_ = std::make_unique<DataProcessor>();
        math_utils_ = std::make_unique<MathUtils>();
        file_parser_ = std::make_unique<FileParsingWrapper>();
        chunker_ = std::make_unique<LeafraChunker>();
        tokenizer_ = std::make_unique<SentencePieceTokenizer>();
        
#ifdef LEAFRA_HAS_TENSORFLOWLITE
        // Initialize TensorFlow Lite variables
        tf_model_ = nullptr;
        tf_interpreter_ = nullptr;
        tf_options_ = nullptr;
        tf_initialized_ = false;
#endif
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
        
        // Initialize SentencePiece tokenizer if enabled
        if (pImpl->tokenizer_ && config.tokenizer.enable_sentencepiece) {
            LEAFRA_INFO() << "Initializing SentencePiece tokenizer";
            
            if (!config.tokenizer.sentencepiece_model_path.empty()) {
                bool loaded = pImpl->tokenizer_->load_model(config.tokenizer.sentencepiece_model_path);
                if (loaded) {
                    LEAFRA_INFO() << "✅ SentencePiece model loaded from: " << config.tokenizer.sentencepiece_model_path;
                    LEAFRA_INFO() << "  - Vocabulary size: " << pImpl->tokenizer_->get_vocab_size();
                } else {
                    LEAFRA_WARNING() << "⚠️  Failed to load SentencePiece model from: " << config.tokenizer.sentencepiece_model_path;                    
                }
            } else {
                LEAFRA_WARNING() << "⚠️  SentencePiece enabled but no model path specified";
            }
        } else if (config.tokenizer.enable_sentencepiece) {
            LEAFRA_WARNING() << "⚠️  SentencePiece requested but tokenizer not available";
        }

#ifdef LEAFRA_HAS_TENSORFLOWLITE
        // Initialize TensorFlow Lite embedding model if enabled
        if (config.embedding_inference.is_valid()) {
            LEAFRA_INFO() << "Initializing TensorFlow Lite embedding model";
            LEAFRA_INFO() << "  - Framework: " << config.embedding_inference.framework;
            LEAFRA_INFO() << "  - Model path: " << config.embedding_inference.model_path;
            
            // Check if model file exists
            std::ifstream model_file(config.embedding_inference.model_path, std::ios::binary);
            if (!model_file.good()) {
                LEAFRA_ERROR() << "❌ TensorFlow Lite model file not found: " << config.embedding_inference.model_path;
                return ResultCode::ERROR_INITIALIZATION_FAILED;
            }
            model_file.close();
            
            // Load the model
            pImpl->tf_model_ = TfLiteModelCreateFromFile(config.embedding_inference.model_path.c_str());
            if (!pImpl->tf_model_) {
                LEAFRA_ERROR() << "❌ Failed to load TensorFlow Lite model from: " << config.embedding_inference.model_path;
                return ResultCode::ERROR_INITIALIZATION_FAILED;
            }
            
            // Create interpreter options
            pImpl->tf_options_ = TfLiteInterpreterOptionsCreate();
            
            // Set number of threads
            if (config.embedding_inference.num_threads > 0) {
                TfLiteInterpreterOptionsSetNumThreads(pImpl->tf_options_, config.embedding_inference.num_threads);
                LEAFRA_DEBUG() << "  - Threads: " << config.embedding_inference.num_threads;
            } else {
                // Use SDK-level thread configuration or default
                int threads = config.max_threads > 0 ? config.max_threads : 4;
                TfLiteInterpreterOptionsSetNumThreads(pImpl->tf_options_, threads);
                LEAFRA_DEBUG() << "  - Threads: " << threads << " (auto)";
            }
            
            // Note: Delegates are disabled for now to simplify initial integration
            // They will be added in a future update once basic functionality is working
            LEAFRA_INFO() << "  - Delegates: DISABLED (will be added in future update)";
            
            // Create interpreter
            pImpl->tf_interpreter_ = TfLiteInterpreterCreate(pImpl->tf_model_, pImpl->tf_options_);
            if (!pImpl->tf_interpreter_) {
                LEAFRA_ERROR() << "❌ Failed to create TensorFlow Lite interpreter";
                return ResultCode::ERROR_INITIALIZATION_FAILED;
            }
            
            // Allocate tensors
            TfLiteStatus allocate_status = TfLiteInterpreterAllocateTensors(pImpl->tf_interpreter_);
            if (allocate_status != kTfLiteOk) {
                LEAFRA_ERROR() << "❌ Failed to allocate tensors for TensorFlow Lite interpreter";
                return ResultCode::ERROR_INITIALIZATION_FAILED;
            }
            
            // Log model information
            int32_t input_count = TfLiteInterpreterGetInputTensorCount(pImpl->tf_interpreter_);
            int32_t output_count = TfLiteInterpreterGetOutputTensorCount(pImpl->tf_interpreter_);
            
            LEAFRA_INFO() << "✅ TensorFlow Lite model initialized successfully";
            LEAFRA_INFO() << "  - Input tensors: " << input_count;
            LEAFRA_INFO() << "  - Output tensors: " << output_count;
            LEAFRA_INFO() << "  - Delegates: " << pImpl->tf_delegates_.size();
            
            pImpl->tf_initialized_ = true;
        } else if (config.embedding_inference.enabled) {
            LEAFRA_WARNING() << "⚠️  Embedding model inference enabled but configuration is invalid";
            LEAFRA_WARNING() << "    Framework: '" << config.embedding_inference.framework << "'";
            LEAFRA_WARNING() << "    Model path: '" << config.embedding_inference.model_path << "'";
        }
#else
        if (config.embedding_inference.enabled) {
            LEAFRA_WARNING() << "⚠️  TensorFlow Lite embedding model requested but not available (library not linked)";
        }
#endif
        
        pImpl->initialized_ = true;
        LEAFRA_INFO() << "LeafraSDK initialized successfully";
        
#ifdef LEAFRA_HAS_PDFIUM
        LEAFRA_INFO() << "✅ PDFium integration: ENABLED";
#else
        LEAFRA_WARNING() << "⚠️  PDFium integration: DISABLED (library not found)";
#endif

#ifdef LEAFRA_HAS_TENSORFLOWLITE
        LEAFRA_INFO() << "✅ TensorFlow Lite integration: ENABLED";
#else
        LEAFRA_WARNING() << "⚠️  TensorFlow Lite integration: DISABLED (library not found)";
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
#ifdef LEAFRA_HAS_TENSORFLOWLITE
        // Cleanup TensorFlow Lite resources
        if (pImpl->tf_initialized_) {
            LEAFRA_DEBUG() << "Shutting down TensorFlow Lite";
            
            // Clean up interpreter
            if (pImpl->tf_interpreter_) {
                TfLiteInterpreterDelete(pImpl->tf_interpreter_);
                pImpl->tf_interpreter_ = nullptr;
            }
            
            // Clean up delegates
            for (TfLiteDelegate* delegate : pImpl->tf_delegates_) {
                if (delegate) {
                    // Note: Delegates are automatically cleaned up when interpreter is deleted,
                    // but we can explicitly delete them if needed for specific delegate types
#if defined(__APPLE__)
                    // For CoreML and Metal delegates, they are automatically cleaned up
#endif
                }
            }
            pImpl->tf_delegates_.clear();
            
            // Clean up options
            if (pImpl->tf_options_) {
                TfLiteInterpreterOptionsDelete(pImpl->tf_options_);
                pImpl->tf_options_ = nullptr;
            }
            
            // Clean up model
            if (pImpl->tf_model_) {
                TfLiteModelDelete(pImpl->tf_model_);
                pImpl->tf_model_ = nullptr;
            }
            
            pImpl->tf_initialized_ = false;
            LEAFRA_DEBUG() << "TensorFlow Lite shutdown completed";
        }
#endif
        
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
                    ResultCode chunk_result = pImpl->chunker_->chunk_document(pages, pImpl->chunker_->get_default_options(), chunks);
                    
                    if (chunk_result == ResultCode::SUCCESS) {
                        LEAFRA_INFO() << "✅ Successfully created " << chunks.size() << " chunks";
                        pImpl->send_event("🧩 Created " + std::to_string(chunks.size()) + " chunks");
                        
                        // Use SentencePiece for accurate token counting if available
                        size_t total_actual_tokens = 0;
                        bool using_sentencepiece = false;
                        
                        if (pImpl->config_.tokenizer.enable_sentencepiece && 
                            pImpl->tokenizer_ && 
                            pImpl->tokenizer_->is_loaded()) {
                            
                            using_sentencepiece = true;
                            LEAFRA_DEBUG() << "Using SentencePiece for accurate token counting";
                            
                            // Get accurate token counts for each chunk
                            for (auto& chunk : chunks) {
                                //Note: chunk.content is a string_view, so we need to convert it to a string
                                //This can be prevented for other models which don't require changing the text by overloading the encode_as_ids with string_view
                                std::string chunk_text = std::string(chunk.content);
                                // Add "passage: " prefix for multilingual-e5-small model per 
                                // https://huggingface.co/intfloat/multilingual-e5-small 
                                if (pImpl->config_.tokenizer.model_name == "multilingual-e5-small") {
                                    chunk_text = "passage: " + chunk_text;
                                }
                                std::vector<int> token_ids = pImpl->tokenizer_->encode_as_ids(chunk_text, SentencePieceTokenizer::TokenizeOptions());
                                //AD TODO: DON'T exceed 512 tokens per chunk otherwise retrieval will be less accurate 
                                //AD TODO trim the token_ids to 512 if it's greater than 512
                                //AD TODO: pad the token_ids to 512 exactly 
                                //load the executorch model and run it on the token_ids
                                // Store both the token count and the actual token IDs


                                
                                chunk.estimated_tokens = token_ids.size(); // Replace estimate with actual count
                                chunk.token_ids = std::move(token_ids);    // Store the actual token IDs
                                total_actual_tokens += chunk.estimated_tokens;
                                
                                // Debug log for first few chunks
                                if (pImpl->config_.debug_mode && &chunk - &chunks[0] < 3) {
                                    LEAFRA_DEBUG() << "Chunk " << (&chunk - &chunks[0] + 1) 
                                                 << " - Characters: " << chunk_text.length() 
                                                 << ", Actual tokens: " << chunk.estimated_tokens
                                                 << ", Token IDs stored: " << chunk.token_ids.size()
                                                 << ", Chars/token ratio: " << (chunk.estimated_tokens > 0 ? static_cast<double>(chunk_text.length()) / chunk.estimated_tokens : 0.0);
                                }
                            }
                            
                            LEAFRA_INFO() << "✅ SentencePiece tokenization completed";
                            LEAFRA_INFO() << "  - Total actual tokens: " << total_actual_tokens;
                            LEAFRA_INFO() << "  - Chunks with token IDs: " << chunks.size();
                            LEAFRA_DEBUG() << "✅ Token IDs stored for all " << chunks.size() << " chunks";
                            
                        } else if (pImpl->config_.tokenizer.enable_sentencepiece) {
                            LEAFRA_WARNING() << "SentencePiece requested but not available, using estimates";
                        }
                        
                        // Calculate statistics (using actual tokens if available, estimates otherwise)
                        size_t total_chunk_chars = 0;
                        size_t total_tokens = 0;
                        for (const auto& chunk : chunks) {
                            total_chunk_chars += chunk.content.length();
                            total_tokens += chunk.estimated_tokens;
                        }
                        
                        LEAFRA_INFO() << "Chunk statistics:";
                        LEAFRA_INFO() << "  - Total chunks: " << chunks.size();
                        LEAFRA_INFO() << "  - Total characters in chunks: " << total_chunk_chars;
                        LEAFRA_INFO() << "  - " << (using_sentencepiece ? "Actual" : "Estimated") << " tokens: " << total_tokens;
                        LEAFRA_INFO() << "  - Avg chunk size: " << (chunks.size() > 0 ? total_chunk_chars / chunks.size() : 0) << " chars";
                        LEAFRA_INFO() << "  - Avg tokens per chunk: " << (chunks.size() > 0 ? total_tokens / chunks.size() : 0);
                        if (using_sentencepiece && total_tokens > 0) {
                            LEAFRA_INFO() << "  - Actual chars/token ratio: " << static_cast<double>(total_chunk_chars) / total_tokens;
                        }
                        
                        std::string token_type = using_sentencepiece ? "actual" : "estimated";
                        pImpl->send_event("📊 Chunks: " + std::to_string(chunks.size()) + 
                                        ", Avg size: " + std::to_string(chunks.size() > 0 ? total_chunk_chars / chunks.size() : 0) + " chars, " +
                                        std::to_string(chunks.size() > 0 ? total_tokens / chunks.size() : 0) + " " + token_type + " tokens");
                        
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
                                LEAFRA_INFO() << "  🔤 Tokens: " << chunk.estimated_tokens << " (" << (using_sentencepiece ? "actual" : "estimated") << ")";
                                if (chunk.has_token_ids()) {
                                    LEAFRA_INFO() << "  🔢 Token IDs: " << chunk.token_ids.size() << " stored";
                                }
                                LEAFRA_INFO() << "  📄 Page: " << (chunk.page_number + 1);
                                LEAFRA_INFO() << "  📍 Position: " << chunk.start_index << "-" << chunk.end_index;
                                if (using_sentencepiece && chunk.estimated_tokens > 0) {
                                    LEAFRA_INFO() << "  📊 Chars/token ratio: " << static_cast<double>(chunk.content.length()) / chunk.estimated_tokens;
                                }
                                LEAFRA_INFO() << "Content:";
                                
                                // Print content based on print mode
                                if (pImpl->config_.chunking.print_chunks_full) {
                                    // Print full content - convert string_view to string to prevent UTF-8 streaming issues
                                    LEAFRA_INFO() << std::string(chunk.content);
                                    
                                    // Print full token IDs if available
                                    if (chunk.has_token_ids() && !chunk.token_ids.empty()) {
                                        LEAFRA_INFO() << "🔢 Token IDs (" << chunk.token_ids.size() << " tokens):";
                                        std::ostringstream token_stream;
                                        for (size_t k = 0; k < chunk.token_ids.size(); ++k) {
                                            token_stream << chunk.token_ids[k];
                                            if (k < chunk.token_ids.size() - 1) token_stream << " ";
                                        }
                                        LEAFRA_INFO() << token_stream.str();
                                    }
                                } else if (pImpl->config_.chunking.print_chunks_brief) {
                                    // Print first N lines
                                    std::istringstream stream(std::string(chunk.content));
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
                                    
                                    // Print brief token IDs if available
                                    if (chunk.has_token_ids() && !chunk.token_ids.empty()) {
                                        const size_t max_tokens_to_show = 20; // Show first 20 token IDs
                                        LEAFRA_INFO() << "🔢 Token IDs (" << chunk.token_ids.size() << " tokens):";
                                        std::ostringstream token_stream;
                                        
                                        size_t tokens_to_display = std::min(max_tokens_to_show, chunk.token_ids.size());
                                        for (size_t k = 0; k < tokens_to_display; ++k) {
                                            token_stream << chunk.token_ids[k];
                                            if (k < tokens_to_display - 1) token_stream << " ";
                                        }
                                        
                                        if (chunk.token_ids.size() > max_tokens_to_show) {
                                            token_stream << " ... (showing first " << max_tokens_to_show << " of " << chunk.token_ids.size() << " tokens)";
                                        }
                                        
                                        LEAFRA_INFO() << token_stream.str();
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
                                std::string content_preview = std::string(chunks[i].content);
                                if (content_preview.length() > 100) {
                                    content_preview = content_preview.substr(0, 100) + "...";
                                } else {
                                    content_preview += "...";
                                }
                                LEAFRA_DEBUG() << "Chunk " << (i + 1) << " (page " << chunks[i].page_number + 1 
                                             << ", " << chunks[i].content.length() << " chars, " 
                                             << chunks[i].estimated_tokens << " tokens): "
                                             << content_preview;
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

std::vector<ChunkTokenInfo> LeafraCore::extract_chunk_token_info(const std::vector<TextChunk>& chunks) {
    std::vector<ChunkTokenInfo> token_info;
    token_info.reserve(chunks.size());
    
    for (size_t i = 0; i < chunks.size(); ++i) {
        const auto& chunk = chunks[i];
        token_info.emplace_back(
            i,                              // chunk_index
            chunk.content,                  // content (string_view)
            chunk.token_ids,                // token_ids (copy the vector)
            chunk.content.length(),         // character_count
            chunk.estimated_tokens,         // token_count (actual token count when SentencePiece is used)
            chunk.page_number               // page_number
        );
    }
    
    return token_info;
}

shared_ptr<LeafraCore> LeafraCore::create() {
    return std::make_shared<LeafraCore>();
}

} // namespace leafra 