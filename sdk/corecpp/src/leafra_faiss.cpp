#include "leafra/leafra_faiss.h"

#ifdef LEAFRA_HAS_FAISS

#include "leafra/logger.h"
#include "leafra/leafra_sqlite.h"
#include <faiss/IndexFlat.h>
#include <faiss/IndexIVFFlat.h>
#include <faiss/IndexIVFPQ.h>
#include <faiss/IndexHNSW.h>  // Now supported with OpenMP xcframework
#include <faiss/IndexLSH.h>
#include <faiss/IndexIDMap.h>
#include <faiss/index_io.h>
#include <faiss/impl/io.h>
#include <faiss/MetricType.h>
#include <faiss/utils/utils.h>
#include <stdexcept>
#include <algorithm>

namespace leafra {

class FaissIndex::Impl {
public:
    std::unique_ptr<faiss::Index> index_;
    std::unique_ptr<faiss::IndexIDMap> id_map_index_;
    int dimension_;
    IndexType index_type_;
    MetricType metric_type_;
    bool use_id_map_;
    
    Impl(int dimension, IndexType index_type, MetricType metric_type)
        : dimension_(dimension), index_type_(index_type), metric_type_(metric_type), use_id_map_(false) {
        
        // Convert metric type
        faiss::MetricType faiss_metric;
        switch (metric_type) {
            case MetricType::L2:
                faiss_metric = faiss::METRIC_L2;
                break;
            case MetricType::INNER_PRODUCT:
                faiss_metric = faiss::METRIC_INNER_PRODUCT;
                break;
            case MetricType::COSINE:
                // FAISS doesn't have native cosine, use inner product with normalized vectors
                faiss_metric = faiss::METRIC_INNER_PRODUCT;
                break;
            default:
                throw std::invalid_argument("Unsupported metric type");
        }
        
        // Create index based on type
        switch (index_type) {
            case IndexType::FLAT:
                index_ = std::make_unique<faiss::IndexFlat>(dimension, faiss_metric);
                break;
                
            case IndexType::IVF_FLAT: {
                // Create quantizer and IVF index
                auto quantizer = std::make_unique<faiss::IndexFlat>(dimension, faiss_metric);
                int nlist = std::max(1, static_cast<int>(std::sqrt(10000))); // Default nlist
                index_ = std::make_unique<faiss::IndexIVFFlat>(quantizer.release(), dimension, nlist, faiss_metric);
                break;
            }
            
            case IndexType::IVF_PQ: {
                // Create quantizer and IVF-PQ index
                auto quantizer = std::make_unique<faiss::IndexFlat>(dimension, faiss_metric);
                int nlist = std::max(1, static_cast<int>(std::sqrt(10000))); // Default nlist
                int m = std::max(1, dimension / 8); // Number of subquantizers
                int nbits = 8; // Bits per subquantizer
                index_ = std::make_unique<faiss::IndexIVFPQ>(quantizer.release(), dimension, nlist, m, nbits, faiss_metric);
                break;
            }
            
            case IndexType::HNSW: {
                // Create HNSW index with default parameters
                int M = 16; // Number of bi-directional links for each node
                index_ = std::make_unique<faiss::IndexHNSWFlat>(dimension, M, faiss_metric);
                break;
            }
            
            case IndexType::LSH: {
                int nbits = std::max(8, dimension / 2); // Number of hash bits
                index_ = std::make_unique<faiss::IndexLSH>(dimension, nbits);
                break;
            }
            
            default:
                throw std::invalid_argument("Unsupported index type");
        }
        
        if (!index_) {
            throw std::runtime_error("Failed to create FAISS index");
        }
        
        // Enable ID mapping by default
        enable_id_map();
    }
    
    void enable_id_map() {
        if (!use_id_map_) {
            id_map_index_ = std::make_unique<faiss::IndexIDMap>(index_.release());
            use_id_map_ = true;
        }
    }
    
    faiss::Index* get_index() {
        return use_id_map_ ? static_cast<faiss::Index*>(id_map_index_.get()) : index_.get();
    }
    
    const faiss::Index* get_index() const {
        return use_id_map_ ? static_cast<const faiss::Index*>(id_map_index_.get()) : index_.get();
    }
};

FaissIndex::FaissIndex(int dimension, IndexType index_type, MetricType metric)
    : pImpl(std::make_unique<Impl>(dimension, index_type, metric)) {
    
    if (dimension <= 0) {
        throw std::invalid_argument("Dimension must be positive");
    }
    
    LEAFRA_INFO() << "Created FAISS index: " << get_index_type_string() 
                  << " (dim=" << dimension << ", metric=" << get_metric_type_string() << ", id_map=enabled)";
}

FaissIndex::~FaissIndex() = default;

FaissIndex::FaissIndex(FaissIndex&& other) noexcept = default;
FaissIndex& FaissIndex::operator=(FaissIndex&& other) noexcept = default;

ResultCode FaissIndex::add_vectors(const float* vectors, int count) {
    if (!vectors || count <= 0) {
        LEAFRA_ERROR() << "Invalid vectors or count";
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    try {
        pImpl->get_index()->add(count, vectors);
        LEAFRA_DEBUG() << "Added " << count << " vectors to FAISS index";
        return ResultCode::SUCCESS;
    } catch (const std::exception& e) {
        LEAFRA_ERROR() << "Failed to add vectors to FAISS index: " << e.what();
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

ResultCode FaissIndex::add_vectors_with_ids(const float* vectors, const int64_t* ids, int count) {
    if (!vectors || !ids || count <= 0) {
        LEAFRA_ERROR() << "Invalid vectors, ids, or count";
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    try {
        // ID mapping is enabled by default in constructor
        pImpl->id_map_index_->add_with_ids(count, vectors, ids);
        LEAFRA_DEBUG() << "Added " << count << " vectors with IDs to FAISS index";
        return ResultCode::SUCCESS;
    } catch (const std::exception& e) {
        LEAFRA_ERROR() << "Failed to add vectors with IDs to FAISS index: " << e.what();
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

ResultCode FaissIndex::search(const float* query_vector, int k, std::vector<SearchResult>& results) {
    if (!query_vector || k <= 0) {
        LEAFRA_ERROR() << "Invalid query vector or k";
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    try {
        std::vector<float> distances(k);
        std::vector<faiss::idx_t> labels(k);
        
        pImpl->get_index()->search(1, query_vector, k, distances.data(), labels.data());
        
        results.clear();
        results.reserve(k);
        
        for (int i = 0; i < k; ++i) {
            if (labels[i] >= 0) { // Valid result
                results.emplace_back(labels[i], distances[i]);
            }
        }
        
        LEAFRA_DEBUG() << "FAISS search found " << results.size() << " results";
        return ResultCode::SUCCESS;
    } catch (const std::exception& e) {
        LEAFRA_ERROR() << "FAISS search failed: " << e.what();
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

ResultCode FaissIndex::batch_search(const float* query_vectors, int query_count, int k, 
                                   std::vector<std::vector<SearchResult>>& results) {
    if (!query_vectors || query_count <= 0 || k <= 0) {
        LEAFRA_ERROR() << "Invalid query vectors, query_count, or k";
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    try {
        std::vector<float> distances(query_count * k);
        std::vector<faiss::idx_t> labels(query_count * k);
        
        pImpl->get_index()->search(query_count, query_vectors, k, distances.data(), labels.data());
        
        results.clear();
        results.resize(query_count);
        
        for (int q = 0; q < query_count; ++q) {
            results[q].clear();
            results[q].reserve(k);
            
            for (int i = 0; i < k; ++i) {
                int idx = q * k + i;
                if (labels[idx] >= 0) { // Valid result
                    results[q].emplace_back(labels[idx], distances[idx]);
                }
            }
        }
        
        LEAFRA_DEBUG() << "FAISS batch search completed for " << query_count << " queries";
        return ResultCode::SUCCESS;
    } catch (const std::exception& e) {
        LEAFRA_ERROR() << "FAISS batch search failed: " << e.what();
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

ResultCode FaissIndex::train(const float* training_vectors, int training_count) {
    if (!training_vectors || training_count <= 0) {
        LEAFRA_ERROR() << "Invalid training vectors or count";
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    try {
        if (!pImpl->get_index()->is_trained) {
            pImpl->get_index()->train(training_count, training_vectors);
            LEAFRA_INFO() << "FAISS index trained with " << training_count << " vectors";
        } else {
            LEAFRA_DEBUG() << "FAISS index already trained";
        }
        return ResultCode::SUCCESS;
    } catch (const std::exception& e) {
        LEAFRA_ERROR() << "FAISS training failed: " << e.what();
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

ResultCode FaissIndex::save_index(const std::string& filename) {
    if (filename.empty()) {
        LEAFRA_ERROR() << "Invalid filename";
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    try {
        faiss::write_index(pImpl->get_index(), filename.c_str());
        LEAFRA_INFO() << "FAISS index saved to: " << filename;
        return ResultCode::SUCCESS;
    } catch (const std::exception& e) {
        LEAFRA_ERROR() << "Failed to save FAISS index: " << e.what();
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

ResultCode FaissIndex::load_index(const std::string& filename) {
    if (filename.empty()) {
        LEAFRA_ERROR() << "Invalid filename";
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    try {
        auto loaded_index = std::unique_ptr<faiss::Index>(faiss::read_index(filename.c_str()));
        
        if (!loaded_index) {
            LEAFRA_ERROR() << "Failed to load FAISS index from: " << filename;
            return ResultCode::ERROR_PROCESSING_FAILED;
        }
        
        // Check dimension compatibility
        if (loaded_index->d != pImpl->dimension_) {
            LEAFRA_ERROR() << "Dimension mismatch: expected " << pImpl->dimension_ 
                          << ", got " << loaded_index->d;
            return ResultCode::ERROR_INVALID_PARAMETER;
        }
        
        auto* id_map_ptr = dynamic_cast<faiss::IndexIDMap*>(loaded_index.get());
        if (id_map_ptr) {
            // Index is already an IndexIDMap
            pImpl->id_map_index_ = std::unique_ptr<faiss::IndexIDMap>(
                static_cast<faiss::IndexIDMap*>(loaded_index.release()));
            pImpl->index_.reset();  // Clear the regular index pointer
            pImpl->use_id_map_ = true;
        } else {
            // Index is not an IndexIDMap, wrap it
            pImpl->index_ = std::move(loaded_index);
            pImpl->id_map_index_.reset();
            pImpl->use_id_map_ = false;
            // Re-enable ID mapping
            pImpl->enable_id_map();
        }
        
        LEAFRA_INFO() << "FAISS index loaded from: " << filename;
        return ResultCode::SUCCESS;
    } catch (const std::exception& e) {
        LEAFRA_ERROR() << "Failed to load FAISS index: " << e.what();
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

int64_t FaissIndex::get_count() const {
    return pImpl->get_index()->ntotal;
}

int FaissIndex::get_dimension() const {
    return pImpl->dimension_;
}

bool FaissIndex::is_trained() const {
    return pImpl->get_index()->is_trained;
}

std::string FaissIndex::get_index_type_string() const {
    switch (pImpl->index_type_) {
        case IndexType::FLAT: return "IndexFlat";
        case IndexType::IVF_FLAT: return "IndexIVFFlat";
        case IndexType::IVF_PQ: return "IndexIVFPQ";
        case IndexType::HNSW: return "IndexHNSWFlat";
        case IndexType::LSH: return "IndexLSH";
        default: return "Unknown";
    }
}

std::string FaissIndex::get_metric_type_string() const {
    switch (pImpl->metric_type_) {
        case MetricType::L2: return "L2";
        case MetricType::INNER_PRODUCT: return "InnerProduct";
        case MetricType::COSINE: return "Cosine";
        default: return "Unknown";
    }
}

ResultCode FaissIndex::save_to_db(SQLiteDatabase& db, const std::string& definition) {
    if (definition.empty()) {
        LEAFRA_ERROR() << "Invalid definition string";
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    try {
        // Serialize FAISS index to memory buffer
        faiss::VectorIOWriter writer;
        faiss::write_index(pImpl->get_index(), &writer);
        
        // Convert to blob for SQLite storage
        std::vector<uint8_t> blob_data = std::move(writer.data);
        
        // Prepare SQL statement to insert/update the blob
        std::string sql = "INSERT OR REPLACE INTO faissindextable (definition, faissdata) VALUES (?, ?)";
        auto stmt = db.prepare(sql);
        
        if (!stmt || !stmt->isValid()) {
            LEAFRA_ERROR() << "Failed to prepare SQL statement for FAISS index save";
            return ResultCode::ERROR_PROCESSING_FAILED;
        }
        
        // Bind parameters
        if (!stmt->bindText(1, definition) || !stmt->bindBlob(2, blob_data)) {
            LEAFRA_ERROR() << "Failed to bind parameters for FAISS index save";
            return ResultCode::ERROR_PROCESSING_FAILED;
        }
        
        // Execute the statement
        if (!stmt->execute()) {
            LEAFRA_ERROR() << "Failed to save FAISS index to database: " << db.getLastErrorMessage();
            return ResultCode::ERROR_PROCESSING_FAILED;
        }
        
        LEAFRA_INFO() << "FAISS index saved to database with definition: " << definition 
                      << " (size: " << blob_data.size() << " bytes)";
        return ResultCode::SUCCESS;
        
    } catch (const std::exception& e) {
        LEAFRA_ERROR() << "Failed to save FAISS index to database: " << e.what();
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

ResultCode FaissIndex::restore_from_db(SQLiteDatabase& db, const std::string& definition) {
    if (definition.empty()) {
        LEAFRA_ERROR() << "Invalid definition string";
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    try {
        // Prepare SQL statement to retrieve the blob
        std::string sql = "SELECT faissdata FROM faissindextable WHERE definition = ?";
        auto stmt = db.prepare(sql);
        
        if (!stmt || !stmt->isValid()) {
            LEAFRA_ERROR() << "Failed to prepare SQL statement for FAISS index restore";
            return ResultCode::ERROR_PROCESSING_FAILED;
        }
        
        // Bind parameter
        if (!stmt->bindText(1, definition)) {
            LEAFRA_ERROR() << "Failed to bind parameter for FAISS index restore";
            return ResultCode::ERROR_PROCESSING_FAILED;
        }
        
        // Execute and get result
        if (!stmt->step()) {
            LEAFRA_ERROR() << "FAISS index not found in database with definition: " << definition;
            return ResultCode::ERROR_NOT_FOUND;
        }
        
        // Get the blob data
        auto row = stmt->getCurrentRow();
        std::vector<uint8_t> blob_data = row.getBlob(0);
        
        if (blob_data.empty()) {
            LEAFRA_ERROR() << "Empty FAISS index data in database";
            return ResultCode::ERROR_PROCESSING_FAILED;
        }
        
        // Deserialize FAISS index from memory buffer
        faiss::VectorIOReader reader;
        reader.data = std::move(blob_data);
        
        auto loaded_index = std::unique_ptr<faiss::Index>(faiss::read_index(&reader));
        
        if (!loaded_index) {
            LEAFRA_ERROR() << "Failed to deserialize FAISS index from database";
            return ResultCode::ERROR_PROCESSING_FAILED;
        }
        
        // Check dimension compatibility
        if (loaded_index->d != pImpl->dimension_) {
            LEAFRA_ERROR() << "Dimension mismatch: expected " << pImpl->dimension_ 
                          << ", got " << loaded_index->d;
            return ResultCode::ERROR_INVALID_PARAMETER;
        }
        
        auto* id_map_ptr = dynamic_cast<faiss::IndexIDMap*>(loaded_index.get());
        if (id_map_ptr) {
            // Index is already an IndexIDMap
            pImpl->id_map_index_ = std::unique_ptr<faiss::IndexIDMap>(
                static_cast<faiss::IndexIDMap*>(loaded_index.release()));
            pImpl->index_.reset();  // Clear the regular index pointer
            pImpl->use_id_map_ = true;
        } else {
            // Index is not an IndexIDMap, wrap it
            pImpl->index_ = std::move(loaded_index);
            pImpl->id_map_index_.reset();
            pImpl->use_id_map_ = false;
            // Re-enable ID mapping
            pImpl->enable_id_map();
        }
        
        LEAFRA_INFO() << "FAISS index restored from database with definition: " << definition
                      << " (vectors: " << pImpl->get_index()->ntotal << ")";
        return ResultCode::SUCCESS;
        
    } catch (const std::exception& e) {
        LEAFRA_ERROR() << "Failed to restore FAISS index from database: " << e.what();
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

ResultCode FaissIndex::remove_vectors(const int64_t* ids, int count) {
    if (!ids || count <= 0) {
        LEAFRA_ERROR() << "Invalid IDs or count";
        return ResultCode::ERROR_INVALID_PARAMETER;
    }
    
    try {
        // ID mapping is enabled by default in constructor
        if (!pImpl->id_map_index_) {
            LEAFRA_ERROR() << "ID mapping not available for vector removal";
            return ResultCode::ERROR_PROCESSING_FAILED;
        }
        
        // FAISS IndexIDMap supports remove_ids with IDSelector
        faiss::IDSelectorArray selector(count, ids);
        pImpl->id_map_index_->remove_ids(selector);
        
        LEAFRA_DEBUG() << "Removed " << count << " vectors from FAISS index";
        return ResultCode::SUCCESS;
    } catch (const std::exception& e) {
        LEAFRA_ERROR() << "Failed to remove vectors from FAISS index: " << e.what();
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

} // namespace leafra

#endif // LEAFRA_HAS_FAISS 