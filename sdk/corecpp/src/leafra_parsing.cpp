#include "leafra/leafra_parsing.h"
#include "leafra/logger.h"
#include "leafra/leafra_debug.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <cctype>

namespace leafra {

// ==============================================================================
// ParsedDocument Implementation
// ==============================================================================

std::string ParsedDocument::getAllText() const {
    std::ostringstream result;
    for (size_t i = 0; i < pages.size(); ++i) {
        if (i > 0) result << "\n\n--- Page " << (i + 1) << " ---\n\n";
        result << pages[i];
    }
    return result.str();
}

bool ParsedDocument::hasMetadata(const std::string& key) const {
    return metadata.find(key) != metadata.end();
}

std::string ParsedDocument::getMetadata(const std::string& key, const std::string& defaultValue) const {
    auto it = metadata.find(key);
    return (it != metadata.end()) ? it->second : defaultValue;
}

// ==============================================================================
// FileParsingWrapper Implementation
// ==============================================================================

FileParsingWrapper::FileParsingWrapper() : initialized_(false) {
    LEAFRA_DEBUG() << "FileParsingWrapper created";
}

FileParsingWrapper::~FileParsingWrapper() {
    shutdown();
    LEAFRA_DEBUG() << "FileParsingWrapper destroyed";
}

bool FileParsingWrapper::initialize() {
    if (initialized_) {
        return true;
    }
    
    LEAFRA_INFO() << "Initializing FileParsingWrapper";
    
    // Register all available adapters
    registerAdapter(std::make_unique<PDFParsingAdapter>());
    registerAdapter(std::make_unique<TextParsingAdapter>());
    registerAdapter(std::make_unique<DOCXParsingAdapter>());
    registerAdapter(std::make_unique<ExcelParsingAdapter>());
    
    initialized_ = true;
    
    LEAFRA_INFO() << "FileParsingWrapper initialized with " << adapters_.size() << " adapters";
    
    // Log supported extensions
    auto extensions = getSupportedExtensions();
    std::ostringstream extList;
    for (size_t i = 0; i < extensions.size(); ++i) {
        if (i > 0) extList << ", ";
        extList << extensions[i];
    }
    LEAFRA_INFO() << "Supported file extensions: " << extList.str();
    
    return true;
}

void FileParsingWrapper::shutdown() {
    if (initialized_) {
        LEAFRA_INFO() << "Shutting down FileParsingWrapper";
        adapters_.clear();
        initialized_ = false;
    }
}

void FileParsingWrapper::registerAdapter(std::unique_ptr<IFileParsingAdapter> adapter) {
    if (adapter) {
        LEAFRA_DEBUG() << "Registering adapter: " << adapter->getName();
        adapters_.push_back(std::move(adapter));
    }
}

ParsedDocument FileParsingWrapper::parseFile(const std::string& filePath) const {
    LEAFRA_DEBUG_TIMER("parseFile");
    
    if (!initialized_) {
        ParsedDocument result;
        result.filePath = filePath;
        result.errorMessage = "FileParsingWrapper not initialized";
        LEAFRA_ERROR() << result.errorMessage;
        LEAFRA_DEBUG_LOG("ERROR", "FileParsingWrapper not initialized for file: " + filePath);
        return result;
    }
    
    auto start_time = debug::timer::now();
    
    LEAFRA_INFO() << "Parsing file: " << filePath;
    LEAFRA_DEBUG_LOG("PARSING", "Starting file parsing: " + filePath);
    
    // Get file size for performance metrics
    size_t file_size = 0;
    try {
        std::filesystem::path path(filePath);
        if (std::filesystem::exists(path)) {
            file_size = std::filesystem::file_size(path);
            LEAFRA_DEBUG_LOG("FILE_INFO", "File size: " + std::to_string(file_size) + " bytes");
        }
    } catch (const std::exception& e) {
        LEAFRA_DEBUG_LOG("WARNING", "Could not get file size: " + std::string(e.what()));
    }
    
    // Find appropriate adapter
    auto adapter_start = debug::timer::now();
    const IFileParsingAdapter* adapter = getAdapterForFile(filePath);
    auto adapter_find_end = debug::timer::now();
    double adapter_find_ms = debug::timer::elapsed_milliseconds(adapter_start, adapter_find_end);
    LEAFRA_DEBUG_LOG("TIMING", "Adapter selection took " + std::to_string(adapter_find_ms) + "ms");
    
    if (!adapter) {
        ParsedDocument result;
        result.filePath = filePath;
        result.errorMessage = "No adapter found for file type: " + extractFileExtension(filePath);
        LEAFRA_WARNING() << result.errorMessage;
        LEAFRA_DEBUG_LOG("ERROR", result.errorMessage);
        return result;
    }
    
    LEAFRA_DEBUG() << "Using adapter: " << adapter->getName();
    LEAFRA_DEBUG_LOG("ADAPTER", "Selected adapter: " + std::string(adapter->getName()) + " for file: " + filePath);
    
    // Parse the file with timing
    auto parse_start = debug::timer::now();
    ParsedDocument result = adapter->parse(filePath);
    auto parse_end = debug::timer::now();
    double parse_ms = debug::timer::elapsed_milliseconds(parse_start, parse_end);
    
    // Calculate total time
    auto total_end = debug::timer::now();
    double total_ms = debug::timer::elapsed_milliseconds(start_time, total_end);
    
    // Log detailed performance metrics
    if (debug::is_debug_enabled()) {
        LEAFRA_DEBUG_LOG("TIMING", "Core parsing took " + std::to_string(parse_ms) + "ms");
        
        if (result.isValid) {
            size_t total_text_length = 0;
            for (const auto& page : result.pages) {
                total_text_length += page.length();
            }
            
            std::string perf_msg = "File parsing completed - Pages: " + std::to_string(result.pages.size()) + 
                                  ", Text length: " + std::to_string(total_text_length) + " chars, " +
                                  "Duration: " + std::to_string(total_ms) + "ms";
            
            if (file_size > 0) {
                double mb_per_sec = (static_cast<double>(file_size) / (1024.0 * 1024.0)) / (total_ms / 1000.0);
                perf_msg += ", Speed: " + std::to_string(mb_per_sec) + " MB/sec";
            }
            
            if (total_text_length > 0) {
                double chars_per_sec = static_cast<double>(total_text_length) / (total_ms / 1000.0);
                perf_msg += ", Text extraction: " + std::to_string(chars_per_sec) + " chars/sec";
            }
            
            LEAFRA_DEBUG_LOG("PERFORMANCE", perf_msg);
        } else {
            LEAFRA_DEBUG_LOG("ERROR", "File parsing failed: " + result.errorMessage + " (Duration: " + std::to_string(total_ms) + "ms)");
        }
    }
    
    return result;
}

std::vector<std::string> FileParsingWrapper::getSupportedExtensions() const {
    std::vector<std::string> allExtensions;
    
    for (const auto& adapter : adapters_) {
        auto adapterExtensions = adapter->getSupportedExtensions();
        allExtensions.insert(allExtensions.end(), adapterExtensions.begin(), adapterExtensions.end());
    }
    
    return allExtensions;
}

const IFileParsingAdapter* FileParsingWrapper::getAdapterForFile(const std::string& filePath) const {
    std::string extension = normalizeExtension(extractFileExtension(filePath));
    
    for (const auto& adapter : adapters_) {
        if (adapter->canHandle(extension)) {
            return adapter.get();
        }
    }
    
    return nullptr;
}

bool FileParsingWrapper::isFileTypeSupported(const std::string& filePath) const {
    return getAdapterForFile(filePath) != nullptr;
}

std::string FileParsingWrapper::extractFileExtension(const std::string& filePath) const {
    std::filesystem::path path(filePath);
    std::string extension = path.extension().string();
    
    // Remove the leading dot
    if (!extension.empty() && extension[0] == '.') {
        extension = extension.substr(1);
    }
    
    return extension;
}

std::string FileParsingWrapper::normalizeExtension(const std::string& extension) const {
    std::string normalized = extension;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
    return normalized;
}

} // namespace leafra 