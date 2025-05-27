#include "leafra/leafra_parsing.h"
#include "leafra/logger.h"

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
    if (!initialized_) {
        ParsedDocument result;
        result.filePath = filePath;
        result.errorMessage = "FileParsingWrapper not initialized";
        LEAFRA_ERROR() << result.errorMessage;
        return result;
    }
    
    LEAFRA_INFO() << "Parsing file: " << filePath;
    
    // Find appropriate adapter
    const IFileParsingAdapter* adapter = getAdapterForFile(filePath);
    if (!adapter) {
        ParsedDocument result;
        result.filePath = filePath;
        result.errorMessage = "No adapter found for file type: " + extractFileExtension(filePath);
        LEAFRA_WARNING() << result.errorMessage;
        return result;
    }
    
    LEAFRA_DEBUG() << "Using adapter: " << adapter->getName();
    
    // Parse the file
    return adapter->parse(filePath);
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