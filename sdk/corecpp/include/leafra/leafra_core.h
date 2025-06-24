#pragma once

#include "types.h"
#include <memory>
#include <functional>

#ifdef LEAFRA_HAS_FAISS
#include "leafra_faiss.h"
#endif

namespace leafra {

// Forward declarations
class DataProcessor;
class FaissIndex;
struct TextChunk;
struct ChunkTokenInfo;

/**
 * @brief Main SDK interface class
 * 
 * This class provides the primary interface for the Leafra SDK.
 * It manages initialization, configuration, and core functionality.
 */
class LEAFRA_API LeafraCore {
public:
    /**
     * @brief Constructor
     */
    LeafraCore();
    
    /**
     * @brief Destructor
     */
    ~LeafraCore();
    
    /**
     * @brief Initialize the SDK with configuration
     * @param config Configuration parameters
     * @return ResultCode indicating success or failure
     */
    ResultCode initialize(const Config& config);
    
    /**
     * @brief Shutdown the SDK and cleanup resources
     * @return ResultCode indicating success or failure
     */
    ResultCode shutdown();
    
    /**
     * @brief Check if SDK is initialized
     * @return true if initialized, false otherwise
     */
    bool is_initialized() const;
    
    /**
     * @brief Get current configuration
     * @return Current configuration
     */
    const Config& get_config() const;
    
    /**
     * @brief Process data using the SDK
     * @param input Input data buffer
     * @param output Output data buffer
     * @return ResultCode indicating success or failure
     */
    ResultCode process_data(const data_buffer_t& input, data_buffer_t& output);
    
    /**
     * @brief Process user files through the SDK
     * @param file_paths Vector of file paths to process
     * @return ResultCode indicating success or failure
     */
    ResultCode process_user_files(const std::vector<std::string>& file_paths);
    
    /**
     * @brief Set event callback
     * @param callback Function to be called on events
     */
    void set_event_callback(callback_t callback);
    
    /**
     * @brief Get SDK version information
     * @return Version string
     */
    static std::string get_version();
    
    /**
     * @brief Get platform information
     * @return Platform string
     */
    static std::string get_platform();
    
    /**
     * @brief Extract chunk token information from processed chunks
     * @param chunks Vector of TextChunk objects with token IDs
     * @return Vector of ChunkTokenInfo for easy access to chunk data and token IDs
     */
    static std::vector<ChunkTokenInfo> extract_chunk_token_info(const std::vector<TextChunk>& chunks);
    
#ifdef LEAFRA_HAS_FAISS
    /**
     * @brief Perform semantic search on processed document chunks
     * @param query Search query string
     * @param max_results Maximum number of results to return
     * @param results Output vector for search results (ID and distance pairs)
     * @return ResultCode indicating success or failure
     */
    ResultCode semantic_search(const std::string& query, int max_results, std::vector<FaissIndex::SearchResult>& results);
    
#ifdef LEAFRA_HAS_LLAMACPP
    /**
     * @brief Perform semantic search with LLM response generation
     * @param query Search query string
     * @param max_results Maximum number of search results to use as context
     * @param results Output vector for search results (populated with found chunks)
     * @param callback Token callback function for streaming LLM response
     * @return ResultCode indicating success or failure
     */
    ResultCode semantic_search_with_llm(const std::string& query, int max_results, std::vector<FaissIndex::SearchResult>& results, token_callback_t callback);
#endif
#endif

#ifdef LEAFRA_HAS_LLAMACPP
    /**
     * @brief Perform LLM inference on a given prompt
     * @param prompt Input prompt text
     * @param response Output response text
     * @return ResultCode indicating success or failure
     */
    ResultCode llm_inference(const std::string& prompt, std::string& response);
#endif
    
    /**
     * @brief Create SDK instance
     * @return Shared pointer to LeafraCore instance
     */
    static shared_ptr<LeafraCore> create();

private:
    class Impl;
    unique_ptr<Impl> pImpl;
    
    // Non-copyable
    LeafraCore(const LeafraCore&) = delete;
    LeafraCore& operator=(const LeafraCore&) = delete;
    
    // Movable
    LeafraCore(LeafraCore&&) = default;
    LeafraCore& operator=(LeafraCore&&) = default;
};

} // namespace leafra 