#include "leafra/leafra_core.h"
#include "leafra/data_processor.h"
#include "leafra/math_utils.h"
#include "leafra/platform_utils.h"
#include "leafra/logger.h"
#include "leafra/leafra_parsing.h"
#include "leafra/leafra_chunker.h"
#include "leafra/leafra_sentencepiece.h"
#include "leafra/leafra_debug.h"
#include "leafra/leafra_filemanager.h"
#include "leafra/leafra_sqlite.h"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

#ifdef LEAFRA_HAS_PDFIUM
#include "fpdfview.h"
#include "fpdf_text.h"
#include "fpdf_doc.h"
#endif

#ifdef LEAFRA_HAS_COREML
// CoreML interface header
#include "leafra/leafra_coreml.h"
#endif

#ifdef LEAFRA_HAS_TENSORFLOWLITE
// TensorFlow Lite C API headers - start with core functionality only
#include "c_api.h"
#include "c_api_types.h"
#include "builtin_ops.h"

// Note: Starting with core TensorFlow Lite functionality only
// Delegates will be added in a future update
#endif

#ifdef LEAFRA_HAS_FAISS
// FAISS interface header for enum conversion methods
#include "leafra/leafra_faiss.h"
#endif

#ifdef LEAFRA_HAS_LLAMACPP
// LlamaCpp interface header
#include "leafra/leafra_llamacpp.h"
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

//TODO AD: Move these to leafra_faiss.cpp 
#ifdef LEAFRA_HAS_FAISS
// Helper functions to convert from config strings to FAISS enum types
static FaissIndex::IndexType get_faiss_index_type_from_string(const std::string& index_type) {
    if (index_type == "FLAT") return FaissIndex::IndexType::FLAT;
    if (index_type == "IVF_FLAT") return FaissIndex::IndexType::IVF_FLAT;
    if (index_type == "IVF_PQ") return FaissIndex::IndexType::IVF_PQ;
    if (index_type == "HNSW") return FaissIndex::IndexType::HNSW;
    if (index_type == "LSH") return FaissIndex::IndexType::LSH;
    return FaissIndex::IndexType::FLAT; // Default fallback
}

static FaissIndex::MetricType get_faiss_metric_type_from_string(const std::string& metric) {
    if (metric == "L2") return FaissIndex::MetricType::L2;
    if (metric == "INNER_PRODUCT") return FaissIndex::MetricType::INNER_PRODUCT;
    if (metric == "COSINE") return FaissIndex::MetricType::COSINE;
    return FaissIndex::MetricType::COSINE; // Default fallback
}
#endif

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
    
#ifdef LEAFRA_HAS_SQLITE
    // SQLite database for document storage
    std::unique_ptr<SQLiteDatabase> database_;
#endif

#ifdef LEAFRA_HAS_FAISS
    // FAISS index for vector search
    std::unique_ptr<FaissIndex> faiss_index_;
#endif
    
#ifdef LEAFRA_HAS_COREML
    // CoreML inference components
    std::unique_ptr<leafra::CoreMLModel> coreml_model_;
    bool coreml_initialized_;
#endif
    
#ifdef LEAFRA_HAS_TENSORFLOWLITE
    // TensorFlow Lite inference components
    TfLiteModel* tf_model_;
    TfLiteInterpreter* tf_interpreter_;
    TfLiteInterpreterOptions* tf_options_;
    std::vector<TfLiteDelegate*> tf_delegates_;
    bool tf_initialized_;
#endif

#ifdef LEAFRA_HAS_LLAMACPP
    // LlamaCpp inference components
    std::unique_ptr<leafra::llamacpp::LlamaCppModel> llamacpp_model_;
    bool llamacpp_initialized_;
#endif
    
    Impl() : initialized_(false), event_callback_(nullptr) {
        data_processor_ = std::make_unique<DataProcessor>();
        math_utils_ = std::make_unique<MathUtils>();
        file_parser_ = std::make_unique<FileParsingWrapper>();
        chunker_ = std::make_unique<LeafraChunker>();
        tokenizer_ = std::make_unique<SentencePieceTokenizer>();
        
#ifdef LEAFRA_HAS_SQLITE
        database_ = std::make_unique<SQLiteDatabase>();
#endif
        
#ifdef LEAFRA_HAS_COREML
        // Initialize CoreML variables
        coreml_model_ = nullptr;
        coreml_initialized_ = false;
#endif
        
#ifdef LEAFRA_HAS_TENSORFLOWLITE
        // Initialize TensorFlow Lite variables
        tf_model_ = nullptr;
        tf_interpreter_ = nullptr;
        tf_options_ = nullptr;
        tf_initialized_ = false;
#endif

#ifdef LEAFRA_HAS_LLAMACPP
        // Initialize LlamaCpp variables
        llamacpp_model_ = nullptr;
        llamacpp_initialized_ = false;
#endif
    }
    
    void send_event(const std::string& message) {
        if (event_callback_) {
            event_callback_(message.c_str());
        }
    }
    
#ifdef LEAFRA_HAS_SQLITE
    /**
     * @brief Insert document and its chunks into the database
     * @param result Parsed document data
     * @param chunks Vector of text chunks with embeddings
     * @param file_path Original file path
     * @return true if successful, false otherwise
     */
    bool insertDocumentAndChunksIntoDatabase(const ParsedDocument& result, 
                                            const std::vector<TextChunk>& chunks, 
                                            const std::string& file_path) {
        if (!database_ || !database_->isOpen()) {
            LEAFRA_ERROR() << "Database not available for document insertion";
            return false;
        }
        
        try {
            // Start transaction for atomic operation
            SQLiteTransaction transaction(*database_);
            
            // Insert document into docs table
            auto insertDocStmt = database_->prepare(
                "INSERT INTO docs (filename, url, creation_date, size) VALUES (?, ?, CURRENT_TIMESTAMP, ?)"
            );
            
            if (!insertDocStmt || !insertDocStmt->isValid()) {
                LEAFRA_ERROR() << "Failed to prepare document insert statement";
                return false;
            }
            
            // Get canonical path (resolves .., ., symlinks) and extract filename
            std::filesystem::path path(file_path);
            std::string absolute_path;
            try {
                // First try canonical (requires file to exist and resolves all components)
                absolute_path = std::filesystem::canonical(path).string();
            } catch (const std::filesystem::filesystem_error& e) {
                LEAFRA_DEBUG() << "Failed to get canonical path for: " << file_path << " - " << e.what();
                return false;
            }
            std::string filename = path.filename().string();
            LEAFRA_DEBUG() << "Filename: " << filename;
            LEAFRA_DEBUG() << "Absolute path: " << absolute_path;

            // Calculate total document size (sum of all page text lengths)
            size_t total_size = 0;
            for (const auto& page : result.pages) {
                total_size += page.length();
            }
            
            // Check if document already exists and delete it if found
            if (!handleExistingDocument(filename, absolute_path)) {
                LEAFRA_ERROR() << "Failed to handle existing document: " << filename;
                            return false;
            }

            // Bind parameters for document
            insertDocStmt->bindText(1, filename);
            insertDocStmt->bindText(2, absolute_path);  // Use absolute path as URL
            insertDocStmt->bindInt64(3, static_cast<long long>(total_size));
            
            // Execute document insert
            if (!insertDocStmt->execute()) {
                LEAFRA_ERROR() << "Failed to insert document: " << filename;
                LEAFRA_ERROR() << "SQLite error code: " << database_->getLastErrorCode();
                LEAFRA_ERROR() << "SQLite error message: " << database_->getLastErrorMessage();
                LEAFRA_DEBUG() << "Attempted to insert - filename: '" << filename << "', url: '" << absolute_path << "', size: " << total_size;
                return false;
            }
            
            // Get the document ID
            long long doc_id = database_->getLastInsertRowId();
            LEAFRA_DEBUG() << "Inserted document with ID: " << doc_id;
            
            // Prepare chunk insert statement
            auto insertChunkStmt = database_->prepare(
                "INSERT INTO chunks (doc_id, chunk_page_number, chunk_faiss_id, chunk_no, chunk_token_size, chunk_size, chunk_text, chunk_embedding) VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
            );
            
            if (!insertChunkStmt || !insertChunkStmt->isValid()) {
                LEAFRA_ERROR() << "Failed to prepare chunk insert statement";
                return false;
            }
            
            // Insert each chunk (only chunks with embeddings)
            size_t chunks_inserted = 0;
            size_t chunks_skipped = 0;
            for (size_t i = 0; i < chunks.size(); ++i) {
                const auto& chunk = chunks[i];
                
                // Skip chunks without embeddings - don't insert them into database
                if (!chunk.has_embedding() || chunk.embedding.empty()) {
                    chunks_skipped++;
                    LEAFRA_WARNING() << "Skipping database insertion for chunk " << (i + 1) << " - no embedding";
                    continue;
                }
                
                // Reset statement for reuse
                insertChunkStmt->reset();
                
                // Convert embedding to blob
                std::vector<uint8_t> embedding_blob;
                    // Convert float vector to byte vector
                    const float* float_data = chunk.embedding.data();
                //TODO AD: We need to handle different endianness'es here per architecture to be portable 
                    const uint8_t* byte_data = reinterpret_cast<const uint8_t*>(float_data);
                    size_t byte_size = chunk.embedding.size() * sizeof(float);
                    embedding_blob.assign(byte_data, byte_data + byte_size);
                
                // Calculate FAISS ID for this chunk (same as used in FAISS insertion)
                
                // TODO AD: There can be overflow here if doc_id is too large
                int64_t chunk_faiss_id = doc_id * 1000000 + static_cast<int64_t>(i);
                
                // Bind parameters for chunk
                insertChunkStmt->bindInt64(1, doc_id);
                insertChunkStmt->bindInt64(2, static_cast<long long>(chunk.page_number+1)); //1-based page number
                insertChunkStmt->bindInt64(3, chunk_faiss_id); // chunk_faiss_id - always present for inserted chunks
                insertChunkStmt->bindInt64(4, static_cast<long long>(i+1)); // chunk_no (1-based)
                insertChunkStmt->bindInt64(5, static_cast<long long>(chunk.estimated_tokens)); // chunk_token_size
                insertChunkStmt->bindInt64(6, static_cast<long long>(chunk.content.length())); // chunk_size
                insertChunkStmt->bindText(7, std::string(chunk.content)); // Convert string_view to string
                insertChunkStmt->bindBlob(8, embedding_blob); // embedding - always present for inserted chunks
                
                // Execute chunk insert
                if (!insertChunkStmt->execute()) {
                    LEAFRA_ERROR() << "Failed to insert chunk " << (i + 1) << " for document: " << filename;
                    return false;
                }
                chunks_inserted++;
            }
            
            // Log insertion summary
            if (chunks_skipped > 0) {
                LEAFRA_INFO() << "Database insertion: " << chunks_inserted << "/" << chunks.size() << " chunks inserted (" << chunks_skipped << " skipped - no embeddings)";
            } else {
                LEAFRA_INFO() << "Database insertion: " << chunks_inserted << "/" << chunks.size() << " chunks inserted";
            }
            
            // Commit transaction
            if (!transaction.commit()) {
                LEAFRA_ERROR() << "Failed to commit document and chunks transaction";
                return false;
            }
    #ifdef LEAFRA_HAS_FAISS
            // Insert chunk embeddings into FAISS index
            if (!insertChunkEmbeddingsIntoFaiss(chunks, doc_id)) {
                LEAFRA_ERROR() << "Failed to insert embeddings into FAISS index for document: " << filename;
                // TODO AD: Consider rolling back the database transaction here
                return false;
            }
    #endif // LEAFRA_HAS_FAISS
            LEAFRA_INFO() << "✅ Successfully inserted document '" << filename << "' with " << chunks.size() << " chunks";
            send_event("💾 Stored document: " + filename + " (" + std::to_string(chunks.size()) + " chunks)");
            
            return true;
            
        } catch (const std::exception& e) {
            LEAFRA_ERROR() << "Exception during document insertion: " << e.what();
            return false;
        }
    } //insertDocumentAndChunksIntoDatabase
#endif // LEAFRA_HAS_SQLITE

    /**
     * @brief Print detailed chunk content analysis for debugging/development
     * @param chunks Vector of text chunks to analyze
     * @param file_path Original file path for context
     * @param using_sentencepiece Whether SentencePiece tokenization was used
     */
    void printChunkContentAnalysis(const std::vector<TextChunk>& chunks, 
                                  const std::string& file_path, 
                                  bool using_sentencepiece) {
        if (!config_.chunking.print_chunks_full && !config_.chunking.print_chunks_brief) {
            return; // Nothing to print
        }
        
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
            if (chunk.has_embedding()) {
                LEAFRA_INFO() << "  🧠 Embedding: " << chunk.embedding.size() << " dimensions";
            }
            LEAFRA_INFO() << "  📄 Page: " << (chunk.page_number + 1);
            LEAFRA_INFO() << "  📍 Position: " << chunk.start_index << "-" << chunk.end_index;
            if (using_sentencepiece && chunk.estimated_tokens > 0) {
                LEAFRA_INFO() << "  📊 Chars/token ratio: " << static_cast<double>(chunk.content.length()) / chunk.estimated_tokens;
            }
            LEAFRA_INFO() << "Content:";
            
            // Print content based on print mode
            if (config_.chunking.print_chunks_full) {
                printFullChunkContent(chunk);
            } else if (config_.chunking.print_chunks_brief) {
                printBriefChunkContent(chunk);
            }
            
            if (i < chunks.size() - 1) {
                LEAFRA_INFO() << "";
            }
        }
        
        LEAFRA_INFO() << "============================================================";
    } //printChunkContentAnalysis
    
    /**
     * @brief Print full chunk content with all details
     * @param chunk Text chunk to print
     */
    void printFullChunkContent(const TextChunk& chunk) {
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
        
        // Print full sentence embedding if available
        if (chunk.has_embedding() && !chunk.embedding.empty()) {
            LEAFRA_INFO() << "🧠 Sentence Embedding (" << chunk.embedding.size() << " dimensions):";
            std::ostringstream embedding_stream;
            embedding_stream << std::fixed << std::setprecision(8);
            embedding_stream << "[";
            for (size_t k = 0; k < chunk.embedding.size(); ++k) {
                embedding_stream << chunk.embedding[k];
                if (k < chunk.embedding.size() - 1) embedding_stream << " ";
            }
            embedding_stream << "]";
            LEAFRA_INFO() << embedding_stream.str();
        }
    } //printFullChunkContent
    
    /**
     * @brief Print brief chunk content with truncation
     * @param chunk Text chunk to print
     */
    void printBriefChunkContent(const TextChunk& chunk) {
        // Print first N lines
        std::istringstream stream(std::string(chunk.content));
        std::string line;
        int line_count = 0;
        int max_lines = config_.chunking.max_lines;
        
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
        
        // Print brief sentence embedding if available
        if (chunk.has_embedding() && !chunk.embedding.empty()) {
            const size_t max_dims_to_show = 10; // Show first 10 dimensions
            LEAFRA_INFO() << "🧠 Sentence Embedding (" << chunk.embedding.size() << " dimensions):";
            std::ostringstream embedding_stream;
            embedding_stream << std::fixed << std::setprecision(8);
            embedding_stream << "[";
            
            size_t dims_to_display = std::min(max_dims_to_show, chunk.embedding.size());
            for (size_t k = 0; k < dims_to_display; ++k) {
                embedding_stream << chunk.embedding[k];
                if (k < dims_to_display - 1) embedding_stream << " ";
            }
            
            if (chunk.embedding.size() > max_dims_to_show) {
                embedding_stream << " ... (showing first " << max_dims_to_show << " of " << chunk.embedding.size() << " dimensions)";
            }
            embedding_stream << "]";
            
            LEAFRA_INFO() << embedding_stream.str();
        }
    } //printBriefChunkContent
    
    /**
     * @brief Print debug chunk summary for development
     * @param chunks Vector of text chunks to summarize
     */
    void printDebugChunkSummary(const std::vector<TextChunk>& chunks) {
        if (!config_.debug_mode || chunks.empty()) {
            return;
        }
        
        size_t chunks_to_log = std::min(size_t(3), chunks.size());
        for (size_t i = 0; i < chunks_to_log; ++i) {
            const auto& chunk = chunks[i];
            std::string content_preview = std::string(chunk.content);
            if (content_preview.length() > 100) {
                content_preview = content_preview.substr(0, 100) + "...";
            } else {
                content_preview += "...";
            }
            LEAFRA_DEBUG() << "Chunk " << (i + 1) << " (page " << chunk.page_number + 1 
                         << ", " << chunk.content.length() << " chars, " 
                         << chunk.estimated_tokens << " tokens): "
                         << content_preview;
        }
        } //printDebugChunkSummary

#ifdef LEAFRA_HAS_COREML
    /**
     * @brief Process chunks through CoreML embedding model
     * @param chunks Vector of text chunks to process (modified in-place with embeddings)
     * @param file_path Original file path for logging context
     * @return Number of successful embeddings generated
     */
    size_t processChunksWithCoreMLEmbeddings(std::vector<TextChunk>& chunks, 
                                           const std::string& file_path) {
        if (!coreml_initialized_ || !coreml_model_) {
            if (config_.embedding_inference.enabled && config_.embedding_inference.framework == "coreml") {
                LEAFRA_WARNING() << "CoreML embedding model requested but not initialized";
            }
            return 0;
        }
        
        LEAFRA_DEBUG_TIMER("coreml_embedding_inference");
        auto start_time = debug::timer::now();
        
        LEAFRA_INFO() << "Starting CoreML embedding inference for " << chunks.size() << " chunks";
        send_event("🧠 Starting embedding inference for " + std::to_string(chunks.size()) + " chunks");
        
        size_t successful_embeddings = 0;
        
        try {
            // Get model input requirements once (cache for performance)
            size_t model_input_count = coreml_model_->getInputCount();
            if (model_input_count != 2) {
                LEAFRA_ERROR() << "Unexpected model input count: " << model_input_count << " (expected 2 for embedding model)";
                return 0; // Skip embedding processing
            }
            
            size_t required_input_size = coreml_model_->getInputSize(0);
            int pad_token = tokenizer_->pad_id();
            if (pad_token < 0) {
                pad_token = 0; // Default to 0 if pad_id is disabled (-1)
            }
            
            LEAFRA_DEBUG() << "CoreML model expects " << required_input_size << " tokens per input";
            LEAFRA_DEBUG() << "Using pad_token: " << pad_token;
            
            // Pre-allocate vectors for reuse (performance optimization)
            std::vector<int> processed_token_ids;
            std::vector<float> input_tokens, attention_mask;
            processed_token_ids.reserve(required_input_size);
            input_tokens.reserve(required_input_size);
            attention_mask.reserve(required_input_size);
            
            // Process each chunk individually
            for (size_t chunk_idx = 0; chunk_idx < chunks.size(); ++chunk_idx) {
                auto& chunk = chunks[chunk_idx];
                
                if (chunk.has_token_ids() && !chunk.token_ids.empty()) {
                    try {
                        successful_embeddings += processChunkEmbedding(chunk, chunk_idx, 
                                                                     required_input_size, pad_token,
                                                                     processed_token_ids, input_tokens, attention_mask);
                    } catch (const std::exception& e) {
                        LEAFRA_ERROR() << "CoreML embedding inference failed for chunk " << (chunk_idx + 1) << ": " << e.what();
                    }
                } else {
                    LEAFRA_DEBUG() << "Skipping chunk " << (chunk_idx + 1) << " - no token IDs available";
                }
            }
            
            LEAFRA_INFO() << "✅ CoreML embedding inference completed for file: " << file_path;
            LEAFRA_INFO() << "  - Total chunks processed: " << chunks.size();
            LEAFRA_INFO() << "  - Successful embeddings: " << successful_embeddings;
            LEAFRA_INFO() << "  - Failed embeddings: " << (chunks.size() - successful_embeddings);
            
            // Log performance metrics
            auto end_time = debug::timer::now();
            double total_duration_ms = debug::timer::elapsed_milliseconds(start_time, end_time);
            LEAFRA_DEBUG_LOG("PERFORMANCE", "CoreML embedding inference completed in " + std::to_string(total_duration_ms) + "ms");
            if (successful_embeddings > 0) {
                double avg_ms_per_chunk = total_duration_ms / successful_embeddings;
                LEAFRA_DEBUG_LOG("PERFORMANCE", "Average inference time per chunk: " + std::to_string(avg_ms_per_chunk) + "ms");
                debug::debug_log_performance("coreml_embedding", chunks.size(), successful_embeddings, total_duration_ms);
            }
            
        } catch (const std::exception& e) {
            LEAFRA_ERROR() << "CoreML embedding inference failed for file " << file_path << ": " << e.what();
        }
        
        return successful_embeddings;
    }
    
    /**
     * @brief Process a single chunk for embedding generation
     * @param chunk Text chunk to process (modified in-place with embedding)
     * @param chunk_number 1-based chunk number for logging
     * @param required_input_size Required input size for the model
     * @param pad_token Padding token ID
     * @param processed_token_ids Reusable vector for token processing
     * @param input_tokens Reusable vector for model input tokens
     * @param attention_mask Reusable vector for attention mask
     * @return 1 if embedding was successfully generated, 0 otherwise
     */
    size_t processChunkEmbedding(TextChunk& chunk, 
                               size_t chunk_number,
                               size_t required_input_size, 
                               int pad_token,
                               std::vector<int>& processed_token_ids,
                               std::vector<float>& input_tokens,
                               std::vector<float>& attention_mask) {
        // Reuse pre-allocated vectors (clear and prepare)
        processed_token_ids.clear();
        input_tokens.clear();
        attention_mask.clear();
        
        // Prepare padded/trimmed token sequence
        size_t original_size = chunk.token_ids.size();
        size_t real_token_count = std::min(original_size, required_input_size);
        
        // Copy and resize in one operation
        processed_token_ids.assign(chunk.token_ids.begin(), 
                                  chunk.token_ids.begin() + real_token_count);
        if (processed_token_ids.size() < required_input_size) {
            processed_token_ids.resize(required_input_size, pad_token);
        }
        
        // Convert int tokens to float and create attention mask in single pass
        input_tokens.resize(required_input_size);
        attention_mask.resize(required_input_size);
        
        for (size_t i = 0; i < required_input_size; ++i) {
            input_tokens[i] = static_cast<float>(processed_token_ids[i]);
            attention_mask[i] = (i < real_token_count) ? 1.0f : 0.0f;
        }
        
        // Debug print input tokens and attention mask as vectors
        if (config_.debug_mode) {
            LEAFRA_DEBUG() << "Chunk " << chunk_number << " input_tokens vector (" << input_tokens.size() << " elements):";
            std::ostringstream tokens_stream;
            tokens_stream << "[";
            for (size_t i = 0; i < input_tokens.size(); ++i) {
                tokens_stream << static_cast<int>(input_tokens[i]);
                if (i < input_tokens.size() - 1) tokens_stream << ", ";
            }
            tokens_stream << "]";
            LEAFRA_DEBUG() << tokens_stream.str();
            
            LEAFRA_DEBUG() << "Chunk " << chunk_number << " attention_mask vector (" << attention_mask.size() << " elements):";
            std::ostringstream mask_stream;
            mask_stream << "[";
            for (size_t i = 0; i < attention_mask.size(); ++i) {
                mask_stream << static_cast<int>(attention_mask[i]);
                if (i < attention_mask.size() - 1) mask_stream << ", ";
            }
            mask_stream << "]";
            LEAFRA_DEBUG() << mask_stream.str();
        }
        
        //Our coreml implementation expects inputs in alphabetical order of their names
        //I've added an explicit check by passing input names to the predict function to make
        //sure that the user is not confused when using the model. This of course requires naming
        //the inputs and outputs in the model; which should be our best practice regardless of inference framework
        std::vector<std::vector<float>> model_inputs = {attention_mask, input_tokens};
        
        // Run embedding inference
        LEAFRA_DEBUG() << "Running CoreML embedding inference for chunk " << chunk_number;
        auto inference_start = debug::timer::now();
        
        // Use explicit input names as best practice (works fore5_embedding_model_i512a512_FP32.mlmodelc)
        std::vector<std::vector<float>> embeddings;
        
        if (config_.tokenizer.model_name == "multilingual-e5-small") {
            std::vector<std::string> input_names = {"attention_mask", "input_ids"};
            embeddings = coreml_model_->predict(model_inputs, input_names);
        } else {
            embeddings = coreml_model_->predict(model_inputs);
        }
        
        auto inference_end = debug::timer::now();
        double inference_ms = debug::timer::elapsed_milliseconds(inference_start, inference_end);
        LEAFRA_DEBUG_LOG("TIMING", "Chunk " + std::to_string(chunk_number) + " inference: " + std::to_string(inference_ms) + "ms");
        
        if (!embeddings.empty() && !embeddings[0].empty()) {
            // Store the embedding vector in the chunk
            chunk.embedding = std::move(embeddings[0]);
            LEAFRA_DEBUG() << "Generated embedding with " << chunk.embedding.size() << " dimensions for chunk " << chunk_number;
            
            // Debug print the embedding vector
            if (config_.debug_mode) {
                LEAFRA_DEBUG() << "Chunk " << chunk_number << " embedding vector (" << chunk.embedding.size() << " dimensions):";
                std::ostringstream embedding_stream;
                embedding_stream << std::fixed << std::setprecision(6);
                embedding_stream << "[";
                for (size_t i = 0; i < chunk.embedding.size(); ++i) {
                    embedding_stream << chunk.embedding[i];
                    if (i < chunk.embedding.size() - 1) embedding_stream << ", ";
                }
                embedding_stream << "]";
                LEAFRA_DEBUG() << embedding_stream.str();
            }
            
            return 1; // Success
        } else {
            LEAFRA_WARNING() << "CoreML model produced empty embedding for chunk " << chunk_number;
            return 0; // Failure
        }
    } //processChunkEmbedding
#endif

    /**
     * @brief Process chunks with SentencePiece tokenization for accurate token counting
     * @param chunks Vector of text chunks to process (modified in-place with token IDs)
     * @return Pair of (total_actual_tokens, using_sentencepiece_flag)
     */
    std::pair<size_t, bool> processChunksWithSentencePieceTokenization(std::vector<TextChunk>& chunks, const std::string& prefix) {
        size_t total_actual_tokens = 0;
        bool using_sentencepiece = false;
        
        if (!config_.tokenizer.enabled || !tokenizer_ || !tokenizer_->is_loaded()) {
            LEAFRA_WARNING() << "SentencePiece requested but not available, using estimates";
            return {total_actual_tokens, using_sentencepiece};
        }
        
        using_sentencepiece = true;
        LEAFRA_DEBUG() << "Using SentencePiece for accurate token counting";
        
        // Get accurate token counts for each chunk
        for (size_t chunk_idx = 0; chunk_idx < chunks.size(); ++chunk_idx) {
            auto& chunk = chunks[chunk_idx];
            
            // Note: chunk.content is a string_view, so we need to convert it to a string
            // This can be prevented for other models which don't require changing the text by overloading the encode_as_ids with string_view
             
            std::string chunk_text = std::string(chunk.content);
            
            // Add "passage: or queary" prefix for multilingual-e5-small model per 
            // https://huggingface.co/intfloat/multilingual-e5-small 
            chunk_text = prefix + chunk_text;
            
            std::vector<int> token_ids = tokenizer_->encode_as_ids(chunk_text, SentencePieceTokenizer::TokenizeOptions());
            
            chunk.estimated_tokens = token_ids.size(); // Replace estimate with actual count
            chunk.token_ids = std::move(token_ids);    // Store the actual token IDs
            total_actual_tokens += chunk.estimated_tokens;
            
            // Debug log for first few chunks
            if (config_.debug_mode && chunk_idx < 3) {
                LEAFRA_DEBUG() << "Chunk " << (chunk_idx + 1) 
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
        
        return {total_actual_tokens, using_sentencepiece};
    } //processChunksWithSentencePieceTokenization

    /**
     * @brief Calculate and log chunk statistics with event notifications
     * @param chunks Vector of text chunks to analyze
     * @param using_sentencepiece Whether actual or estimated tokens are being used
     */
    void calculateAndLogChunkStatistics(const std::vector<TextChunk>& chunks, bool using_sentencepiece) {
        // Calculate statistics (using actual tokens if available, estimates otherwise)
        size_t total_chunk_chars = 0;
        size_t total_tokens = 0;
        for (const auto& chunk : chunks) {
            total_chunk_chars += chunk.content.length();
            total_tokens += chunk.estimated_tokens;
        }
        
        // Log detailed statistics
        LEAFRA_INFO() << "Chunk statistics:";
        LEAFRA_INFO() << "  - Total chunks: " << chunks.size();
        LEAFRA_INFO() << "  - Total characters in chunks: " << total_chunk_chars;
        LEAFRA_INFO() << "  - " << (using_sentencepiece ? "Actual" : "Estimated") << " tokens: " << total_tokens;
        LEAFRA_INFO() << "  - Avg chunk size: " << (chunks.size() > 0 ? total_chunk_chars / chunks.size() : 0) << " chars";
        LEAFRA_INFO() << "  - Avg tokens per chunk: " << (chunks.size() > 0 ? total_tokens / chunks.size() : 0);
        
        if (using_sentencepiece && total_tokens > 0) {
            LEAFRA_INFO() << "  - Actual chars/token ratio: " << static_cast<double>(total_chunk_chars) / total_tokens;
        }
        
        // Send summary event with key metrics
        std::string token_type = using_sentencepiece ? "actual" : "estimated";
        send_event("📊 Chunks: " + std::to_string(chunks.size()) + 
                  ", Avg size: " + std::to_string(chunks.size() > 0 ? total_chunk_chars / chunks.size() : 0) + " chars, " +
                  std::to_string(chunks.size() > 0 ? total_tokens / chunks.size() : 0) + " " + token_type + " tokens");
    } //calculateAndLogChunkStatistics
 
#ifdef LEAFRA_HAS_SQLITE
    /**
     * @brief Check if document exists and delete it if found
     * @param filename Document filename
     * @param absolute_path Absolute path to the document
     * @return true if successful (document didn't exist or was successfully deleted), false on error
     */
    bool handleExistingDocument(const std::string& filename, const std::string& absolute_path) {
        if (!database_ || !database_->isOpen()) {
            return false;
        }
        
        auto checkStmt = database_->prepare("SELECT id FROM docs WHERE filename = ? AND url = ?");
        if (!checkStmt || !checkStmt->isValid()) {
            LEAFRA_ERROR() << "Failed to prepare document existence check statement";
            return false;
        }
        
        checkStmt->bindText(1, filename);
        checkStmt->bindText(2, absolute_path);
        
        if (checkStmt->step()) {
            long long existing_doc_id = checkStmt->getCurrentRow().getInt64(0);
            LEAFRA_INFO() << "Document already exists in database: " << filename << " (ID: " << existing_doc_id << ")";
            
#ifdef LEAFRA_HAS_FAISS
            // TODO AD: index types other than flat does not support vector removal, simply return for now for duplicate documents
            // for these other index types we need to reindex or insert document hashes and mark older doc & its chunks invalid in the chunk table
            // need to think this a bit more later, for now we'll simply return if index type is not flat
            // Check if index type is not flat (only flat index supports vector removal)
            if (faiss_index_ && config_.vector_search.index_type != "FLAT") {
                LEAFRA_INFO() << "Skipping document deletion - vector removal only supported for FLAT index type, current: " << config_.vector_search.index_type;
                return true; // Allow document re-insertion without deletion
            }
            // First, collect FAISS IDs of chunks that will be deleted (for FAISS cleanup)
            std::vector<int64_t> faiss_ids_to_remove;
            if (faiss_index_) {
                auto collectFaissIdsStmt = database_->prepare("SELECT chunk_faiss_id FROM chunks WHERE doc_id = ? AND chunk_faiss_id IS NOT NULL");
                if (collectFaissIdsStmt && collectFaissIdsStmt->isValid()) {
                    collectFaissIdsStmt->bindInt64(1, existing_doc_id);
                    while (collectFaissIdsStmt->step()) {
                        int64_t faiss_id = collectFaissIdsStmt->getCurrentRow().getInt64(0);
                        faiss_ids_to_remove.push_back(faiss_id);
                    }
                    LEAFRA_INFO() << "Found " << faiss_ids_to_remove.size() << " FAISS vectors to remove for document: " << filename;
                }
            }
#endif
            
            // Delete existing chunks first (due to foreign key constraint)
            auto deleteChunksStmt = database_->prepare("DELETE FROM chunks WHERE doc_id = ?");
            if (!deleteChunksStmt || !deleteChunksStmt->isValid()) {
                LEAFRA_ERROR() << "Failed to prepare chunk deletion statement";
                return false;
            }
            
            deleteChunksStmt->bindInt64(1, existing_doc_id);
            if (deleteChunksStmt->execute()) {
                int deleted_chunks = database_->getChanges();
                LEAFRA_INFO() << "Deleted " << deleted_chunks << " existing chunks for document: " << filename;
                
#ifdef LEAFRA_HAS_FAISS
                // Remove vectors from FAISS index
                if (faiss_index_ && !faiss_ids_to_remove.empty()) {
                    ResultCode result = faiss_index_->remove_vectors(faiss_ids_to_remove.data(), faiss_ids_to_remove.size());
                    if (result == ResultCode::SUCCESS) {
                        LEAFRA_INFO() << "Removed " << faiss_ids_to_remove.size() << " vectors from FAISS index for document: " << filename;
                    } else {
                        LEAFRA_ERROR() << "Failed to remove vectors from FAISS index for document: " << filename;
                        // Don't return false here - database cleanup succeeded, FAISS cleanup failed but we can continue
                    }
                }
#endif
            } else {
                LEAFRA_ERROR() << "Failed to delete existing chunks for document: " << filename;
                return false;
            }
            
            // Delete the document record
            auto deleteDocStmt = database_->prepare("DELETE FROM docs WHERE id = ?");
            if (!deleteDocStmt || !deleteDocStmt->isValid()) {
                LEAFRA_ERROR() << "Failed to prepare document deletion statement";
                return false;
            }
            
            deleteDocStmt->bindInt64(1, existing_doc_id);
            if (deleteDocStmt->execute()) {
                LEAFRA_INFO() << "Deleted existing document: " << filename << " (ID: " << existing_doc_id << ")";
                send_event("🗑️ Replaced existing document: " + filename);
            } else {
                LEAFRA_ERROR() << "Failed to delete existing document: " << filename;
                return false;
            }
        }
        
        return true; // Success: either document didn't exist or was successfully deleted
    } //handleExistingDocument
#endif // LEAFRA_HAS_SQLITE

#ifdef LEAFRA_HAS_FAISS
    /**
     * @brief Insert chunk embeddings into FAISS index
     * @param chunks Vector of text chunks with embeddings
     * @param doc_id Document ID for generating unique FAISS IDs
     */
    bool insertChunkEmbeddingsIntoFaiss(const std::vector<TextChunk>& chunks, int64_t doc_id) {
        if (!faiss_index_ || !config_.vector_search.enabled) {
            return true; // Not an error if FAISS is disabled
        }
        
        // Reserve space to avoid reallocations
        std::vector<float> embeddings_to_add;
        std::vector<int64_t> chunk_ids;
        
        // Count embeddings and validate - chunks should have embeddings at this point
        size_t embedding_count = 0;
        size_t chunks_without_embeddings = 0;
        size_t embedding_dim = 0;
        
        for (size_t i = 0; i < chunks.size(); ++i) {
            const auto& chunk = chunks[i];
            if (chunk.has_embedding() && !chunk.embedding.empty()) {
                embedding_count++;
                if (embedding_dim == 0) {
                    embedding_dim = chunk.embedding.size();
                } else if (chunk.embedding.size() != embedding_dim) {
                    LEAFRA_ERROR() << "Inconsistent embedding dimension: expected " << embedding_dim << ", got " << chunk.embedding.size();
                    return false; // Return error for inconsistent embeddings
                }
            } else {
                // Chunks are expected to have embeddings at this point
                chunks_without_embeddings++;
                LEAFRA_WARNING() << "Chunk " << i << " missing embedding - skipping FAISS insertion";
            }
        }
        
        // Log warning if some chunks don't have embeddings
        if (chunks_without_embeddings > 0) {
            LEAFRA_WARNING() << "Skipped " << chunks_without_embeddings << " chunks without embeddings out of " << chunks.size() << " total chunks";
        }
        
        if (embedding_count == 0) {
            return true; // No embeddings to add, not an error
        }
        
        // Reserve space for efficiency
        embeddings_to_add.reserve(embedding_count * embedding_dim);
        chunk_ids.reserve(embedding_count);
        
        // Collect embeddings and IDs
        for (size_t i = 0; i < chunks.size(); ++i) {
            const auto& chunk = chunks[i];
            if (chunk.has_embedding() && !chunk.embedding.empty()) {
                // Add embedding data (avoid copy by using insert with iterators)
                embeddings_to_add.insert(embeddings_to_add.end(), 
                                        chunk.embedding.begin(), 
                                        chunk.embedding.end());
                // Use a composite ID: doc_id * 1000000 + chunk_index for uniqueness
                chunk_ids.push_back(doc_id * 1000000 + static_cast<int64_t>(i));
            }
        }
        
        // Add vectors to FAISS index
        auto faiss_result = faiss_index_->add_vectors_with_ids(
            embeddings_to_add.data(), 
            chunk_ids.data(), 
            static_cast<int>(embedding_count)
        );
        
        if (faiss_result == ResultCode::SUCCESS) {
            LEAFRA_INFO() << "✅ Added " << embedding_count << " embeddings to FAISS index (" << embedding_count << "/" << chunks.size() << " chunks)";
            send_event("🔍 Added " + std::to_string(embedding_count) + "/" + std::to_string(chunks.size()) + " embeddings to search index");
            
            // Save updated FAISS index to database
            if (database_ && database_->isOpen()) {
                auto save_result = faiss_index_->save_to_db(*database_, "PrimaryDocEmbeddings");
                if (save_result == ResultCode::SUCCESS) {
                    LEAFRA_DEBUG() << "FAISS index saved to database";
                } else {
                    LEAFRA_WARNING() << "Failed to save FAISS index to database";
                }
            }
            return true;
        } else {
            LEAFRA_ERROR() << "Failed to add embeddings to FAISS index";
            send_event("❌ Failed to add embeddings to search index");
            return false;
        }
    } //insertChunkEmbeddingsIntoFaiss
#endif // LEAFRA_HAS_FAISS
 
}; // LeafraCore::Impl

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
        if (pImpl->tokenizer_ && config.tokenizer.enabled) {
            LEAFRA_INFO() << "Initializing SentencePiece tokenizer";
            
            if (!config.tokenizer.model_path.empty()) {
                bool loaded = pImpl->tokenizer_->load_model(config.tokenizer);
                if (loaded) {
                    LEAFRA_INFO() << "✅ SentencePiece model loaded from: " << config.tokenizer.model_path;
                    LEAFRA_INFO() << "  - Vocabulary size: " << pImpl->tokenizer_->get_vocab_size();
                } else {
                    LEAFRA_WARNING() << "⚠️  Failed to load SentencePiece model from: " << config.tokenizer.model_path;                    
                }
            } else {
                LEAFRA_WARNING() << "⚠️  SentencePiece enabled but no model path specified";
            }
        } else if (config.tokenizer.enabled) {
            LEAFRA_WARNING() << "⚠️  SentencePiece requested but tokenizer not available";
        }

#ifdef LEAFRA_HAS_COREML
        // Initialize CoreML embedding model if enabled
        if (config.embedding_inference.is_valid() && config.embedding_inference.framework == "coreml") {
            LEAFRA_INFO() << "Initializing CoreML embedding model";
            LEAFRA_INFO() << "  - Framework: " << config.embedding_inference.framework;
            LEAFRA_INFO() << "  - Model path: " << config.embedding_inference.model_path;
            LEAFRA_INFO() << "  - Compute units: " << config.embedding_inference.coreml_compute_units;
            
            // Load the CoreML model using the C++ class
            try {
                // Convert compute units string to enum
                leafra::CoreMLModel::ComputeUnits compute_units = leafra::CoreMLModel::ComputeUnits::All;
                if (config.embedding_inference.coreml_compute_units == "cpu") {
                    compute_units = leafra::CoreMLModel::ComputeUnits::CPUOnly;
                } else if (config.embedding_inference.coreml_compute_units == "cpu_and_gpu") {
                    compute_units = leafra::CoreMLModel::ComputeUnits::CPUAndGPU;
                } else if (config.embedding_inference.coreml_compute_units == "cpu_and_neural_engine") {
                    compute_units = leafra::CoreMLModel::ComputeUnits::CPUAndNeuralEngine;
                }
                
                pImpl->coreml_model_ = std::make_unique<leafra::CoreMLModel>(
                    config.embedding_inference.model_path,
                    compute_units
                );
                
                if (pImpl->coreml_model_->isValid()) {
                    // Get model description
                    std::string description = pImpl->coreml_model_->getDescription();
                    if (!description.empty()) {
                        LEAFRA_INFO() << "  - Model description: " << description;
                    }
                    
                    LEAFRA_INFO() << "  - Input count: " << pImpl->coreml_model_->getInputCount();
                    LEAFRA_INFO() << "  - Output count: " << pImpl->coreml_model_->getOutputCount();
                    
                    pImpl->coreml_initialized_ = true;
                    LEAFRA_INFO() << "✅ CoreML model initialized successfully";
                } else {
                    LEAFRA_ERROR() << "❌ CoreML model is not valid";
                    return ResultCode::ERROR_INITIALIZATION_FAILED;
                }
            } catch (const std::exception& e) {
                LEAFRA_ERROR() << "❌ Failed to initialize CoreML model: " << e.what();
                return ResultCode::ERROR_INITIALIZATION_FAILED;
            }
        } else if (config.embedding_inference.enabled && config.embedding_inference.framework == "coreml") {
            LEAFRA_WARNING() << "⚠️  CoreML embedding model enabled but configuration is invalid";
            LEAFRA_WARNING() << "    Framework: '" << config.embedding_inference.framework << "'";
            LEAFRA_WARNING() << "    Model path: '" << config.embedding_inference.model_path << "'";
        }
#else
        if (config.embedding_inference.enabled && config.embedding_inference.framework == "coreml") {
            LEAFRA_WARNING() << "⚠️  CoreML embedding model requested but not available (framework not linked)";
        }
#endif

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
            if (config.embedding_inference.tflite_num_threads > 0) {
                TfLiteInterpreterOptionsSetNumThreads(pImpl->tf_options_, config.embedding_inference.tflite_num_threads);
                LEAFRA_DEBUG() << "  - Threads: " << config.embedding_inference.tflite_num_threads;
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
        
        // Initialize SQLite database - create if necessary 
#ifdef LEAFRA_HAS_SQLITE
        LEAFRA_INFO() << "Initializing SQLite database";
        
        // Get absolute path to the database using FileManager
        std::string db_absolute_path = FileManager::getAbsolutePath(
            StorageType::AppStorage, 
            config.leafra_document_database_name
        );
        
        LEAFRA_DEBUG() << "Database path: " << db_absolute_path;
        
        // Check if database exists
        if (!SQLiteDatabase::fileExists(db_absolute_path)) {
            LEAFRA_INFO() << "Database does not exist, creating new database: " << config.leafra_document_database_name;
            
            // Create the database with RAG schema
            if (SQLiteDatabase::createdb(config.leafra_document_database_name)) {
                LEAFRA_INFO() << "✅ Database created successfully: " << config.leafra_document_database_name;
                pImpl->send_event("Database created: " + config.leafra_document_database_name);
            } else {
                LEAFRA_ERROR() << "❌ Failed to create database: " << config.leafra_document_database_name;
                pImpl->send_event("Failed to create database: " + config.leafra_document_database_name);
                return ResultCode::ERROR_INITIALIZATION_FAILED;
            }
        } else {
            LEAFRA_INFO() << "✅ Database already exists: " << config.leafra_document_database_name;
            pImpl->send_event("Database found: " + config.leafra_document_database_name);
        }
        
        // Open the database using our member object
        if (pImpl->database_) {
            if (pImpl->database_->open(config.leafra_document_database_name)) {
                LEAFRA_INFO() << "✅ Database opened successfully: " << config.leafra_document_database_name;
                pImpl->send_event("Database opened: " + config.leafra_document_database_name);
            } else {
                LEAFRA_ERROR() << "❌ Failed to open database: " << config.leafra_document_database_name;
                pImpl->send_event("Failed to open database: " + config.leafra_document_database_name);
                return ResultCode::ERROR_INITIALIZATION_FAILED;
            }
        }

        // Initialize FAISS index using the sdk config settings
    #ifdef LEAFRA_HAS_FAISS
        if (config.vector_search.enabled) {
            try {
                // Create FAISS index with config settings
                pImpl->faiss_index_ = std::make_unique<FaissIndex>(
                    config.vector_search.dimension,
                    get_faiss_index_type_from_string(config.vector_search.index_type),
                    get_faiss_metric_type_from_string(config.vector_search.metric)
                );
                
                // Restore from database if available
                if (pImpl->database_ && pImpl->database_->isOpen()) {
                    auto restore_result = pImpl->faiss_index_->restore_from_db(*pImpl->database_, "PrimaryDocEmbeddings");
                    if (restore_result == ResultCode::SUCCESS) {
                        LEAFRA_INFO() << "✅ FAISS index restored from database";
                        pImpl->send_event("FAISS index restored from database");
                    } else if (restore_result == ResultCode::ERROR_NOT_FOUND) {
                        LEAFRA_INFO() << "No existing FAISS index found in database - starting fresh";
                        pImpl->send_event("Starting with fresh FAISS index");
                    } else {
                        LEAFRA_ERROR() << "Failed to restore FAISS index from database";
                        pImpl->send_event("Failed to restore FAISS index from database");
                        return ResultCode::ERROR_INITIALIZATION_FAILED;
                    }
                }
            
                LEAFRA_INFO() << "✅ FAISS index initialized successfully";
                pImpl->send_event("FAISS index initialized");
            } catch (const std::exception& e) {
                LEAFRA_ERROR() << "Failed to initialize FAISS index: " << e.what();
                pImpl->send_event("Failed to initialize FAISS index: " + std::string(e.what()));
                return ResultCode::ERROR_INITIALIZATION_FAILED;
            }
        } else {
            LEAFRA_INFO() << "Vector search disabled in configuration";
        }
    #else // LEAFRA_HAS_FAISS
        LEAFRA_WARNING() << "⚠️  FAISS integration: DISABLED (library not found)";
    #endif // LEAFRA_HAS_FAISS
        
#else // LEAFRA_HAS_SQLITE
        LEAFRA_WARNING() << "⚠️  SQLite integration disabled - database initialization skipped";
#endif // LEAFRA_HAS_SQLITE
    
#ifdef LEAFRA_HAS_LLAMACPP
        if (config.llm.enabled) {
            LEAFRA_INFO() << "Initializing LLM inference";
            LEAFRA_INFO() << "  - Framework: " << config.llm.framework;
            LEAFRA_INFO() << "  - Model path: " << config.llm.model_path;
            LEAFRA_INFO() << "  - Context size: " << config.llm.n_ctx;
            LEAFRA_INFO() << "  - Max tokens: " << config.llm.n_predict;
            LEAFRA_INFO() << "  - Temperature: " << config.llm.temperature;
            
            // Initialize LlamaCpp backend
            if (!leafra::llamacpp::global::initialize(false)) {
                LEAFRA_ERROR() << "Failed to initialize LlamaCpp backend";
                return ResultCode::ERROR_INITIALIZATION_FAILED;
            }
            
            // Create LlamaCpp model instance
            pImpl->llamacpp_model_ = std::make_unique<leafra::llamacpp::LlamaCppModel>();
            
            // Load model directly with LLMConfig
            if (!pImpl->llamacpp_model_->load_model(config.llm)) {
                LEAFRA_ERROR() << "Failed to load LlamaCpp model: " << pImpl->llamacpp_model_->get_last_error();
                pImpl->llamacpp_model_.reset();
                leafra::llamacpp::global::cleanup();
                return ResultCode::ERROR_INITIALIZATION_FAILED;
            }
            
            // Set system prompt if provided
            if (!config.llm.system_prompt.empty()) {
                if (!pImpl->llamacpp_model_->set_system_prompt(config.llm.system_prompt)) {
                    LEAFRA_WARNING() << "Failed to set system prompt: " << pImpl->llamacpp_model_->get_last_error();
                }
            }
            
            pImpl->llamacpp_initialized_ = true;
            LEAFRA_INFO() << "✅ LlamaCpp model loaded successfully";
            LEAFRA_INFO() << "  - Model info: " << pImpl->llamacpp_model_->get_model_info();
        }
#endif

        pImpl->initialized_ = true;
        LEAFRA_INFO() << "LeafraSDK initialized successfully";
        
#ifdef LEAFRA_HAS_PDFIUM
        LEAFRA_INFO() << "✅ PDFium integration: ENABLED";
#else
        LEAFRA_WARNING() << "⚠️  PDFium integration: DISABLED (library not found)";
#endif

#ifdef LEAFRA_HAS_COREML
        LEAFRA_INFO() << "✅ CoreML integration: ENABLED";
#else
        LEAFRA_WARNING() << "⚠️  CoreML integration: DISABLED (framework not found)";
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
#ifdef LEAFRA_HAS_COREML
        // Cleanup CoreML resources
        if (pImpl->coreml_initialized_) {
            LEAFRA_DEBUG() << "Shutting down CoreML";
            
            // Destroy the CoreML model
            if (pImpl->coreml_model_) {
                pImpl->coreml_model_.reset();
            }
            
            pImpl->coreml_initialized_ = false;
            
            LEAFRA_DEBUG() << "CoreML shutdown completed";
        }
#endif

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

#ifdef LEAFRA_HAS_LLAMACPP
        // Cleanup LlamaCpp resources
        if (pImpl->llamacpp_initialized_) {
            LEAFRA_DEBUG() << "Shutting down LlamaCpp";
            
            // Unload model and cleanup
            if (pImpl->llamacpp_model_) {
                pImpl->llamacpp_model_->unload();
                pImpl->llamacpp_model_.reset();
            }
            
            // Cleanup global LlamaCpp resources
            leafra::llamacpp::global::cleanup();
            
            pImpl->llamacpp_initialized_ = false;
            LEAFRA_DEBUG() << "LlamaCpp shutdown completed";
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
        
#ifdef LEAFRA_HAS_SQLITE
        // Shutdown database
        if (pImpl->database_ && pImpl->database_->isOpen()) {
            pImpl->database_->close();
            LEAFRA_DEBUG() << "Database shutdown completed";
        }
#endif
        
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
                        std::string prefix;
                        if (pImpl->config_.tokenizer.model_name == "multilingual-e5-small") {
                            prefix = "passage: ";
                        }
                        // Use SentencePiece for accurate token counting if available
                        auto [total_actual_tokens, using_sentencepiece] = pImpl->processChunksWithSentencePieceTokenization(chunks, prefix);

#ifdef LEAFRA_HAS_COREML
                        // Process chunks through CoreML embedding model if available (only if SentencePiece was successful)
                        if (using_sentencepiece) {
                            pImpl->processChunksWithCoreMLEmbeddings(chunks, file_path);
                        }
#endif
                        // Calculate and log chunk statistics
                        pImpl->calculateAndLogChunkStatistics(chunks, using_sentencepiece);
                        // Print detailed chunk content if requested (development/debug feature)
                        pImpl->printChunkContentAnalysis(chunks, file_path, using_sentencepiece);
                        // Optional: Log first few chunks for debugging (only in debug mode)
                        pImpl->printDebugChunkSummary(chunks);
                                          
                        // Insert document and chunks into database
#ifdef LEAFRA_HAS_SQLITE
                        if (pImpl->database_ && pImpl->database_->isOpen()) {
                            LEAFRA_DEBUG() << "Inserting document and chunks into database";
                            if (!pImpl->insertDocumentAndChunksIntoDatabase(result, chunks, file_path)) {
                                LEAFRA_WARNING() << "Failed to insert document into database: " << file_path;
                                pImpl->send_event("⚠️ Database insertion failed for: " + file_path);
                            }
                        } else {
                            LEAFRA_DEBUG() << "Database not available, skipping document insertion";
                        }
#endif
                        //here - insert document into FAISS index 
                        
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
} //process_user_files

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



// Simple Semantic search 
ResultCode LeafraCore::semantic_search(const std::string& query, int max_results, std::vector<FaissIndex::SearchResult>& results) {
    if (!pImpl->initialized_) {
        LEAFRA_ERROR() << "LeafraCore not initialized";
        return ResultCode::ERROR_INITIALIZATION_FAILED;
    }
    
    if (query.empty() || max_results <= 0) {
        LEAFRA_ERROR() << "Invalid query or max_results";
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
#ifdef LEAFRA_HAS_FAISS
    if (!pImpl->faiss_index_) {
        LEAFRA_ERROR() << "FAISS index not available";
        return ResultCode::ERROR_INITIALIZATION_FAILED;
    }
#else
    LEAFRA_ERROR() << "FAISS support not compiled";
    return ResultCode::ERROR_NOT_IMPLEMENTED;
#endif

#ifdef LEAFRA_HAS_COREML
    if (!pImpl->coreml_initialized_ || !pImpl->coreml_model_) {
        LEAFRA_ERROR() << "CoreML model not available";
        return ResultCode::ERROR_INITIALIZATION_FAILED;
    }
#else
    LEAFRA_ERROR() << "CoreML support not compiled";
    return ResultCode::ERROR_NOT_IMPLEMENTED;
#endif

    if (!pImpl->tokenizer_) {
        LEAFRA_ERROR() << "SentencePiece tokenizer not available";
        return ResultCode::ERROR_INITIALIZATION_FAILED;
    }
    
    try {
        
        // Create a single page with the query (using same format as existing code)
        std::vector<std::string> pages;
        pages.push_back(query);
        
        // Create chunks using the existing chunker
        std::vector<TextChunk> chunks;
        ResultCode chunk_result = pImpl->chunker_->chunk_document(pages, pImpl->chunker_->get_default_options(), chunks);
        
        if (chunk_result != ResultCode::SUCCESS || chunks.empty()) {
            LEAFRA_ERROR() << "Failed to create chunks from query";
            return ResultCode::ERROR_PROCESSING_FAILED;
        }
        std::string prefix; 
        if (pImpl->config_.tokenizer.model_name == "multilingual-e5-small") {
            prefix = "query: ";
        }
        // Use SentencePiece for tokenization (reusing existing pipeline)
        auto [total_actual_tokens, using_sentencepiece] = pImpl->processChunksWithSentencePieceTokenization(chunks, prefix);
        
        if (!using_sentencepiece) {
            LEAFRA_ERROR() << "SentencePiece tokenization failed for query";
            return ResultCode::ERROR_PROCESSING_FAILED;
        }
        
#ifdef LEAFRA_HAS_COREML
        // Process through CoreML embedding model (reusing existing pipeline)
        pImpl->processChunksWithCoreMLEmbeddings(chunks, "semantic_search_query");
#endif
        
        // Use only the first chunk for search
        if (chunks.empty() || !chunks[0].has_embedding()) {
            LEAFRA_ERROR() << "No embedding generated for query";
            return ResultCode::ERROR_PROCESSING_FAILED;
        }
        
        const auto& query_embedding = chunks[0].embedding;
        std::ostringstream embedding_stream;
        embedding_stream << "Generated embedding for query (dim: " << query_embedding.size() << "): [";
        for (size_t i = 0; i < query_embedding.size(); ++i) {
            embedding_stream << query_embedding[i];
            if (i < query_embedding.size() - 1) {
                embedding_stream << ", ";
            }
        }
        embedding_stream << "]";
        LEAFRA_DEBUG() << embedding_stream.str();
        
        
#ifdef LEAFRA_HAS_FAISS
        // Perform FAISS search
        ResultCode search_result = pImpl->faiss_index_->search(
            query_embedding.data(), 
            max_results, 
            results
        );
        
        if (search_result != ResultCode::SUCCESS) {
            LEAFRA_ERROR() << "FAISS search failed";
            return search_result;
        }

        // Get the chunks from the database using FAISS IDs
#ifdef LEAFRA_HAS_SQLITE
        if (pImpl->database_ && pImpl->database_->isOpen()) {
            std::vector<FaissIndex::SearchResult> final_results;
            final_results.reserve(results.size());
            
            // Prepare the statement once outside the loop for better performance
            auto stmt = pImpl->database_->prepare(
                "SELECT c.doc_id, c.chunk_no, c.chunk_text, c.chunk_page_number, d.filename "
                "FROM chunks c "
                "JOIN docs d ON c.doc_id = d.id "
                "WHERE c.chunk_faiss_id = ?"
            );
            
            if (stmt && stmt->isValid()) {
                for (const auto& faiss_result : results) {
                    // Bind the FAISS ID parameter for this iteration
                    stmt->bindInt64(1, faiss_result.id);
                    
                    if (stmt->step()) {
                        auto row = stmt->getCurrentRow();
                        
                        // Create a SearchResult with enriched information
                        FaissIndex::SearchResult enriched_result(faiss_result.id, faiss_result.distance);
                        enriched_result.doc_id = row.getInt64(0);
                        enriched_result.chunk_index = row.getInt(1);
                        enriched_result.content = row.getText(2);
                        enriched_result.page_number = row.getInt(3);
                        enriched_result.filename = row.getText(4);
                        
                        final_results.push_back(std::move(enriched_result));
                        
                        // Log the retrieved chunk information for debugging
                        LEAFRA_DEBUG() << "Found chunk - Doc: " << enriched_result.filename 
                                      << ", Page: " << enriched_result.page_number
                                      << ", Chunk: " << enriched_result.chunk_index
                                      << ", Distance: " << enriched_result.distance;
                    } else {
                        LEAFRA_WARNING() << "FAISS ID " << faiss_result.id << " not found in database";
                    }
                    
                    // Reset the statement for the next iteration
                    stmt->reset();
                }
            } else {
                LEAFRA_ERROR() << "Failed to prepare chunk lookup query";
            }
            
            results = std::move(final_results);
            LEAFRA_INFO() << "Semantic search found " << results.size() << " valid results for query";
        } else {
            LEAFRA_WARNING() << "Database not available for chunk lookup";
        }
#else
        LEAFRA_WARNING() << "SQLite support not compiled, returning FAISS IDs only";
#endif
        


        LEAFRA_INFO() << "Semantic search completed with " << results.size() << " results";
        return ResultCode::SUCCESS;
#endif
        
    } catch (const std::exception& e) {
        LEAFRA_ERROR() << "Exception in semantic_search: " << e.what();
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
} //semantic_search

//Semantic Search with LLM 
//This is a simple semantic search that uses the LLM to generate a response to the query - and it streams the results to the user via a callback.
//first it uses the semantic_search to get the most relevant n chunks (max_results)
//then it uses the generate_chat_response_stream to generate a response to the query using the chunks as context
//the response is streamed to the user via the callback
//the callback is a function that takes a string and a bool (is_final)
//the string is the token that is generated by the LLM
//the bool is true if the token is the last token in the response
//the callback is used to stream the response to the user
//internally it uses the semantic_search and generate_chat_response_stream(const std::vector<ChatMessage>& messages, TokenCallback callback, int32_t max_tokens) {
ResultCode LeafraCore::semantic_search_with_llm(const std::string& query, int max_results, std::vector<FaissIndex::SearchResult>& results, token_callback_t callback) {
    if (!pImpl->initialized_) {
        LEAFRA_ERROR() << "LeafraCore not initialized";
        return ResultCode::ERROR_INITIALIZATION_FAILED;
    }
    
    if (query.empty() || max_results <= 0) {
        LEAFRA_ERROR() << "Invalid query or max_results";
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    if (!callback) {
        LEAFRA_ERROR() << "Invalid callback function";
        return ResultCode::ERROR_INVALID_PARAMETER;
    }

#ifdef LEAFRA_HAS_LLAMACPP
    if (!pImpl->llamacpp_initialized_ || !pImpl->llamacpp_model_) {
        LEAFRA_ERROR() << "LlamaCpp not initialized or model not loaded";
        return ResultCode::ERROR_INITIALIZATION_FAILED;
    }
#else
    LEAFRA_ERROR() << "LlamaCpp support not compiled";
    return ResultCode::ERROR_NOT_IMPLEMENTED;
#endif

    try {
        // Step 1: Perform semantic search to get the most relevant chunks
        LEAFRA_DEBUG() << "Performing semantic search for query: " << query.substr(0, 100) << (query.length() > 100 ? "..." : "");
        
        ResultCode search_result = semantic_search(query, max_results, results);
        if (search_result != ResultCode::SUCCESS) {
            LEAFRA_ERROR() << "Semantic search failed";
            return search_result;
        }
        
        if (results.empty()) {
            LEAFRA_WARNING() << "No search results found for query";
            // Still proceed with LLM generation but without context
        }
        
        LEAFRA_INFO() << "Found " << results.size() << " relevant chunks for LLM context";
        
        // Step 2: Build context from search results
        std::ostringstream context_stream;
        context_stream << "Based on the following relevant information:\n\n";
        
        for (size_t i = 0; i < results.size(); ++i) {
            const auto& result = results[i];
            context_stream << "Context " << (i + 1) << " (from " << result.filename 
                          << ", page " << result.page_number << "):\n";
            context_stream << result.content << "\n\n";
        }
        
        context_stream << "Please answer the following question: " << query;
        
        std::string context = context_stream.str();
        LEAFRA_DEBUG() << "Built context for LLM (length: " << context.length() << " chars)";
        
        // Step 3: Create chat messages for the LLM
        std::vector<leafra::llamacpp::ChatMessage> messages;
        
        // Add system prompt if configured
        if (!pImpl->config_.llm.system_prompt.empty()) {
            messages.emplace_back("system", pImpl->config_.llm.system_prompt);
        }
        
        // Add the user query with context
        messages.emplace_back("user", context);
        
        // Step 4: Generate response using LLM with streaming
        LEAFRA_DEBUG() << "Starting LLM generation with streaming callback";
        
#ifdef LEAFRA_HAS_LLAMACPP
        bool generation_success = pImpl->llamacpp_model_->generate_chat_response_stream(
            messages, 
            callback, 
            pImpl->config_.llm.n_predict
        );
        
        if (!generation_success) {
            LEAFRA_ERROR() << "LLM generation failed: " << pImpl->llamacpp_model_->get_last_error();
            return ResultCode::ERROR_PROCESSING_FAILED;
        }
        
        // Log generation statistics
        auto stats = pImpl->llamacpp_model_->get_last_stats();
        LEAFRA_INFO() << "\n Semantic search with LLM completed successfully";
        LEAFRA_INFO() << "  - Search results: " << results.size();
        LEAFRA_INFO() << "  - Prompt tokens: " << stats.prompt_tokens;
        LEAFRA_INFO() << "  - Generated tokens: " << stats.generated_tokens;
        LEAFRA_INFO() << "  - Generation time: " << stats.generation_time << "ms";
        LEAFRA_INFO() << "  - Tokens/second: " << stats.tokens_per_second;
        
        pImpl->send_event("Semantic search with LLM completed - Generated " + std::to_string(stats.generated_tokens) + " tokens");
        
        return ResultCode::SUCCESS;
#endif
        
    } catch (const std::exception& e) {
        LEAFRA_ERROR() << "Exception in semantic_search_with_llm: " << e.what();
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
} //semantic_search_with_llm





//LLM inference functions
//TODO AD: We need to turn this into a chat response - and use the chat template to generate the response.
//Also naming is not great - we need to use a more descriptive name.
ResultCode LeafraCore::llm_inference(const std::string& prompt, std::string& response) {
    if (!pImpl->initialized_) {
        LEAFRA_ERROR() << "LeafraCore not initialized";
        return ResultCode::ERROR_INITIALIZATION_FAILED;
    }
    
    if (prompt.empty()) {
        LEAFRA_ERROR() << "Empty prompt provided";
        return ResultCode::ERROR_INVALID_PARAMETER;
    }

#ifdef LEAFRA_HAS_LLAMACPP
    if (!pImpl->llamacpp_initialized_ || !pImpl->llamacpp_model_) {
        LEAFRA_ERROR() << "LlamaCpp not initialized or model not loaded";
        return ResultCode::ERROR_INITIALIZATION_FAILED;
    }
    
    try {
        LEAFRA_DEBUG() << "Generating response for prompt: " << prompt.substr(0, 100) << (prompt.length() > 100 ? "..." : "");
        
        // Generate response using LlamaCpp
        response = pImpl->llamacpp_model_->generate_text(prompt, pImpl->config_.llm.n_predict);
        if (response.empty()) {
            LEAFRA_ERROR() << "Failed to generate text: " << pImpl->llamacpp_model_->get_last_error();
            return ResultCode::ERROR_PROCESSING_FAILED;
        }
        
        // Log generation statistics
        auto stats = pImpl->llamacpp_model_->get_last_stats();
        LEAFRA_INFO() << "LLM inference completed successfully";
        LEAFRA_INFO() << "  - Prompt tokens: " << stats.prompt_tokens;
        LEAFRA_INFO() << "  - Generated tokens: " << stats.generated_tokens;
        LEAFRA_INFO() << "  - Prompt eval time: " << stats.prompt_eval_time << "ms";
        LEAFRA_INFO() << "  - Generation time: " << stats.generation_time << "ms";
        LEAFRA_INFO() << "  - Tokens/second: " << stats.tokens_per_second;
        
        pImpl->send_event("LLM inference completed - Generated " + std::to_string(stats.generated_tokens) + " tokens");
        
        return ResultCode::SUCCESS;
        
    } catch (const std::exception& e) {
        LEAFRA_ERROR() << "Exception in llm_inference: " << e.what();
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
#else
    LEAFRA_ERROR() << "LlamaCpp support not compiled";
    return ResultCode::ERROR_NOT_IMPLEMENTED;
#endif
} //llm_inference

} // namespace leafra 