#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace leafra {

// Forward declarations
struct ParsedDocument;

// Base interface for file parsing adapters
class IFileParsingAdapter {
public:
    virtual ~IFileParsingAdapter() = default;
    
    // Check if this adapter can handle the given file extension
    virtual bool canHandle(const std::string& extension) const = 0;
    
    // Parse the file and return structured data
    virtual ParsedDocument parse(const std::string& filePath) const = 0;
    
    // Get supported file extensions
    virtual std::vector<std::string> getSupportedExtensions() const = 0;
    
    // Get adapter name for logging
    virtual std::string getName() const = 0;
};

// Parsed document structure
struct ParsedDocument {
    std::string filePath;
    std::string fileType;
    std::string title;
    std::string author;
    std::vector<std::string> pages;  // Text content per page
    std::unordered_map<std::string, std::string> metadata;
    bool isValid = false;
    std::string errorMessage;
    
    // Helper methods
    std::string getAllText() const;
    size_t getPageCount() const { return pages.size(); }
    bool hasMetadata(const std::string& key) const;
    std::string getMetadata(const std::string& key, const std::string& defaultValue = "") const;
};

// PDF parsing adapter using PDFium
class PDFParsingAdapter : public IFileParsingAdapter {
public:
    PDFParsingAdapter();
    ~PDFParsingAdapter();
    
    bool canHandle(const std::string& extension) const override;
    ParsedDocument parse(const std::string& filePath) const override;
    std::vector<std::string> getSupportedExtensions() const override;
    std::string getName() const override;

private:
    bool initializePDFium();
    void shutdownPDFium();
    std::string extractTextFromPage(void* page) const;
    void extractMetadata(void* document, ParsedDocument& result) const;
    
    bool pdfiumInitialized_;
};

// Text file parsing adapter
class TextParsingAdapter : public IFileParsingAdapter {
public:
    bool canHandle(const std::string& extension) const override;
    ParsedDocument parse(const std::string& filePath) const override;
    std::vector<std::string> getSupportedExtensions() const override;
    std::string getName() const override;
};

// DOCX parsing adapter (placeholder for future implementation)
class DOCXParsingAdapter : public IFileParsingAdapter {
public:
    bool canHandle(const std::string& extension) const override;
    ParsedDocument parse(const std::string& filePath) const override;
    std::vector<std::string> getSupportedExtensions() const override;
    std::string getName() const override;
};

// Excel parsing adapter (placeholder for future implementation)
class ExcelParsingAdapter : public IFileParsingAdapter {
public:
    bool canHandle(const std::string& extension) const override;
    ParsedDocument parse(const std::string& filePath) const override;
    std::vector<std::string> getSupportedExtensions() const override;
    std::string getName() const override;
};

// Main parsing wrapper class
class FileParsingWrapper {
public:
    FileParsingWrapper();
    ~FileParsingWrapper();
    
    // Initialize the parsing system
    bool initialize();
    
    // Shutdown and cleanup
    void shutdown();
    
    // Register a parsing adapter
    void registerAdapter(std::unique_ptr<IFileParsingAdapter> adapter);
    
    // Parse a file (automatically detects type from extension)
    ParsedDocument parseFile(const std::string& filePath) const;
    
    // Get list of all supported file extensions
    std::vector<std::string> getSupportedExtensions() const;
    
    // Get adapter for specific file extension
    const IFileParsingAdapter* getAdapterForFile(const std::string& filePath) const;
    
    // Check if file type is supported
    bool isFileTypeSupported(const std::string& filePath) const;

private:
    std::vector<std::unique_ptr<IFileParsingAdapter>> adapters_;
    bool initialized_;
    
    // Helper methods
    std::string extractFileExtension(const std::string& filePath) const;
    std::string normalizeExtension(const std::string& extension) const;
};

} // namespace leafra 