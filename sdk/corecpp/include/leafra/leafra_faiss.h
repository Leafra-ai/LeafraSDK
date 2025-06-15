#pragma once

#include "leafra/platform_utils.h"
#include "leafra/types.h"
#include <vector>
#include <string>
#include <memory>

#ifdef LEAFRA_HAS_FAISS

namespace leafra {

// Forward declaration
class SQLiteDatabase;

/**
 * @brief FAISS vector search wrapper for efficient similarity search
 * 
 * This class provides a simplified interface to FAISS (Facebook AI Similarity Search)
 * for building and querying vector indices. It supports various index types and
 * distance metrics for embedding-based search.
 */
class LEAFRA_API FaissIndex {
public:
    /**
     * @brief Supported FAISS index types
     */
    enum class IndexType {
        FLAT,           // Exact search using L2 distance
        IVF_FLAT,       // Inverted file with exact post-verification
        IVF_PQ,         // Inverted file with product quantization
        HNSW,           // Hierarchical Navigable Small World graphs (now supported with OpenMP)
        LSH             // Locality-Sensitive Hashing
    };
    
    /**
     * @brief Distance metrics supported by FAISS
     */
    enum class MetricType {
        L2,             // Euclidean distance
        INNER_PRODUCT,  // Inner product (for normalized vectors, equivalent to cosine)
        COSINE          // Cosine similarity
    };

    /**
     * @brief Search result containing vector ID and distance, with optional chunk metadata
     */
    struct SearchResult {
        int64_t id;         // Vector ID (FAISS ID)
        float distance;     // Distance/similarity score
        
        // Optional chunk metadata (populated by semantic search)
        int64_t doc_id = -1;        // Document ID from database
        int chunk_index = -1;       // Chunk index within document
        int page_number = -1;       // Page number where chunk appears
        std::string content;        // Chunk text content
        std::string filename;       // Source document filename
        
        SearchResult(int64_t id = -1, float distance = 0.0f) 
            : id(id), distance(distance) {}
    };

    /**
     * @brief Constructor
     * @param dimension Vector dimension
     * @param index_type Type of FAISS index to create
     * @param metric Distance metric to use
     */
    FaissIndex(int dimension, IndexType index_type = IndexType::FLAT, MetricType metric = MetricType::L2);
    
    /**
     * @brief Destructor
     */
    ~FaissIndex();
    
    // Disable copy constructor and assignment operator
    FaissIndex(const FaissIndex&) = delete;
    FaissIndex& operator=(const FaissIndex&) = delete;
    
    // Enable move constructor and assignment operator
    FaissIndex(FaissIndex&& other) noexcept;
    FaissIndex& operator=(FaissIndex&& other) noexcept;

    /**
     * @brief Add vectors to the index
     * @param vectors Vector data (dimension * count floats)
     * @param count Number of vectors to add
     * @return ResultCode indicating success or failure
     */
    ResultCode add_vectors(const float* vectors, int count);
    
    /**
     * @brief Add vectors to the index with explicit IDs
     * @param vectors Vector data (dimension * count floats)
     * @param ids Vector IDs (count int64_t values)
     * @param count Number of vectors to add
     * @return ResultCode indicating success or failure
     */
    ResultCode add_vectors_with_ids(const float* vectors, const int64_t* ids, int count);
    
    /**
     * @brief Search for k nearest neighbors
     * @param query_vector Query vector (dimension floats)
     * @param k Number of nearest neighbors to find
     * @param results Output vector for search results
     * @return ResultCode indicating success or failure
     */
    ResultCode search(const float* query_vector, int k, std::vector<SearchResult>& results);
    
    /**
     * @brief Batch search for multiple query vectors
     * @param query_vectors Query vectors (dimension * query_count floats)
     * @param query_count Number of query vectors
     * @param k Number of nearest neighbors per query
     * @param results Output vector for all search results (query_count * k results)
     * @return ResultCode indicating success or failure
     */
    ResultCode batch_search(const float* query_vectors, int query_count, int k, 
                           std::vector<std::vector<SearchResult>>& results);
    
    /**
     * @brief Train the index (required for some index types like IVF)
     * @param training_vectors Training vectors (dimension * training_count floats)
     * @param training_count Number of training vectors
     * @return ResultCode indicating success or failure
     */
    ResultCode train(const float* training_vectors, int training_count);
    
    /**
     * @brief Save index to file
     * @param filename Path to save the index
     * @return ResultCode indicating success or failure
     */
    ResultCode save_index(const std::string& filename);
    
    /**
     * @brief Load index from file
     * @param filename Path to load the index from
     * @return ResultCode indicating success or failure
     */
    ResultCode load_index(const std::string& filename);
    
    /**
     * @brief Get the number of vectors in the index
     * @return Number of vectors
     */
    int64_t get_count() const;
    
    /**
     * @brief Get the dimension of vectors in the index
     * @return Vector dimension
     */
    int get_dimension() const;
    
    /**
     * @brief Check if the index is trained (applicable for some index types)
     * @return True if trained, false otherwise
     */
    bool is_trained() const;
    
    /**
     * @brief Get index type as string
     * @return String representation of index type
     */
    std::string get_index_type_string() const;
    
    /**
     * @brief Get metric type as string
     * @return String representation of metric type
     */
    std::string get_metric_type_string() const;

    /**
     * @brief Save FAISS index to SQLite database as blob
     * @param db SQLite database reference
     * @param definition Table/field definition string for storage identification
     * @return ResultCode indicating success or failure
     */
    ResultCode save_to_db(SQLiteDatabase& db, const std::string& definition);
    
    /**
     * @brief Restore FAISS index from SQLite database blob
     * @param db SQLite database reference  
     * @param definition Table/field definition string for storage identification
     * @return ResultCode indicating success or failure
     */
    ResultCode restore_from_db(SQLiteDatabase& db, const std::string& definition);

    /**
     * @brief Remove vectors from the index by their IDs
     * @param ids Vector IDs to remove
     * @param count Number of IDs to remove
     * @return ResultCode indicating success or failure
     * @note Not all index types support efficient removal. This method requires ID mapping to be enabled.
     */
    ResultCode remove_vectors(const int64_t* ids, int count);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace leafra

#endif // LEAFRA_HAS_FAISS 