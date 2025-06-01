#include "leafra/leafra_parsing.h"
#include "leafra/logger.h"

#include <algorithm>
#include <cctype>

#ifdef LEAFRA_HAS_PDFIUM
#include "fpdfview.h"
#include "fpdf_text.h"
#include "fpdf_doc.h"
#endif

namespace leafra {

// ==============================================================================
// PDFParsingAdapter Implementation
// ==============================================================================

PDFParsingAdapter::PDFParsingAdapter() : pdfiumInitialized_(false) {
    LEAFRA_DEBUG() << "PDFParsingAdapter created";
}

PDFParsingAdapter::~PDFParsingAdapter() {
    if (pdfiumInitialized_) {
        shutdownPDFium();
    }
    LEAFRA_DEBUG() << "PDFParsingAdapter destroyed";
}

bool PDFParsingAdapter::canHandle(const std::string& extension) const {
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == "pdf";
}

std::vector<std::string> PDFParsingAdapter::getSupportedExtensions() const {
    return {"pdf"};
}

std::string PDFParsingAdapter::getName() const {
    return "PDFParsingAdapter";
}

bool PDFParsingAdapter::initializePDFium() {
#ifdef LEAFRA_HAS_PDFIUM
    if (pdfiumInitialized_) {
        return true;
    }
    
    FPDF_LIBRARY_CONFIG config;
    config.version = 2;
    config.m_pUserFontPaths = nullptr;
    config.m_pIsolate = nullptr;
    config.m_v8EmbedderSlot = 0;
    
    FPDF_InitLibraryWithConfig(&config);
    pdfiumInitialized_ = true;
    
    LEAFRA_INFO() << "PDFium initialized successfully for parsing";
    return true;
#else
    LEAFRA_WARNING() << "PDFium not available - PDF parsing disabled";
    return false;
#endif
}

void PDFParsingAdapter::shutdownPDFium() {
#ifdef LEAFRA_HAS_PDFIUM
    if (pdfiumInitialized_) {
        FPDF_DestroyLibrary();
        pdfiumInitialized_ = false;
        LEAFRA_INFO() << "PDFium shutdown completed";
    }
#endif
}

ParsedDocument PDFParsingAdapter::parse(const std::string& filePath) const {
    ParsedDocument result;
    result.filePath = filePath;
    result.fileType = "PDF";
    
#ifdef LEAFRA_HAS_PDFIUM
    if (!pdfiumInitialized_) {
        // Try to initialize if not already done
        const_cast<PDFParsingAdapter*>(this)->initializePDFium();
        if (!pdfiumInitialized_) {
            result.errorMessage = "PDFium not available";
            return result;
        }
    }
    
    LEAFRA_INFO() << "Parsing PDF file: " << filePath;
    
    // Load the PDF document
    FPDF_DOCUMENT document = FPDF_LoadDocument(filePath.c_str(), nullptr);
    if (!document) {
        unsigned long error = FPDF_GetLastError();
        result.errorMessage = "Failed to load PDF document. Error: " + std::to_string(error);
        LEAFRA_ERROR() << result.errorMessage;
        return result;
    }
    
    // Extract metadata
    extractMetadata(document, result);
    
    // Get page count
    int pageCount = FPDF_GetPageCount(document);
    LEAFRA_INFO() << "PDF has " << pageCount << " pages";
    
    // Extract text from each page
    for (int i = 0; i < pageCount; ++i) {
        FPDF_PAGE page = FPDF_LoadPage(document, i);
        if (page) {
            std::string pageText = extractTextFromPage(page);
            result.pages.push_back(pageText);
            FPDF_ClosePage(page);
            LEAFRA_DEBUG() << "Extracted " << pageText.length() << " characters from page " << (i + 1);
        } else {
            result.pages.push_back("");
            LEAFRA_WARNING() << "Failed to load page " << (i + 1);
        }
    }
    
    FPDF_CloseDocument(document);
    result.isValid = true;
    
    LEAFRA_INFO() << "Successfully parsed PDF with " << result.pages.size() << " pages";
    
#else
    result.errorMessage = "PDFium not available - cannot parse PDF files";
    LEAFRA_WARNING() << result.errorMessage;
#endif
    
    return result;
}

std::string PDFParsingAdapter::extractTextFromPage(void* page) const {
#ifdef LEAFRA_HAS_PDFIUM
    FPDF_PAGE fpdfPage = static_cast<FPDF_PAGE>(page);
    
    // Create text page
    FPDF_TEXTPAGE textPage = FPDFText_LoadPage(fpdfPage);
    if (!textPage) {
        LEAFRA_WARNING() << "Failed to create text page";
        return "";
    }
    
    // Get text length
    int textLength = FPDFText_CountChars(textPage);
    if (textLength <= 0) {
        FPDFText_ClosePage(textPage);
        return "";
    }
    
    // Extract text
    std::vector<unsigned short> buffer(textLength + 1);
    int actualLength = FPDFText_GetText(textPage, 0, textLength, buffer.data());
    
    FPDFText_ClosePage(textPage);
    
    // Convert UTF-16 to UTF-8 properly
    std::string result;
    result.reserve(actualLength * 2); // Reserve space to avoid reallocations
    
    for (int i = 0; i < actualLength - 1; ++i) {
        uint16_t codeUnit = static_cast<uint16_t>(buffer[i]);
        uint32_t codePoint;
        
        // Check for surrogate pairs (UTF-16 encoding for characters > U+FFFF)
        if (codeUnit >= 0xD800 && codeUnit <= 0xDBFF) {
            // High surrogate - need to read low surrogate
            if (i + 1 < actualLength - 1) {
                uint16_t lowSurrogate = static_cast<uint16_t>(buffer[i + 1]);
                if (lowSurrogate >= 0xDC00 && lowSurrogate <= 0xDFFF) {
                    // Valid surrogate pair - combine to get full code point
                    codePoint = 0x10000 + ((codeUnit & 0x3FF) << 10) + (lowSurrogate & 0x3FF);
                    ++i; // Skip the low surrogate in next iteration
                } else {
                    // Invalid surrogate pair
                    codePoint = 0xFFFD; // Unicode replacement character
                }
            } else {
                // Truncated surrogate pair
                codePoint = 0xFFFD; // Unicode replacement character
            }
        } else if (codeUnit >= 0xDC00 && codeUnit <= 0xDFFF) {
            // Lone low surrogate (invalid)
            codePoint = 0xFFFD; // Unicode replacement character
        } else {
            // Regular BMP character
            codePoint = codeUnit;
        }
        
        // Encode code point as UTF-8
        if (codePoint <= 0x7F) {
            // 1-byte sequence (ASCII)
            result += static_cast<char>(codePoint);
        } else if (codePoint <= 0x7FF) {
            // 2-byte sequence
            result += static_cast<char>(0xC0 | (codePoint >> 6));
            result += static_cast<char>(0x80 | (codePoint & 0x3F));
        } else if (codePoint <= 0xFFFF) {
            // 3-byte sequence
            result += static_cast<char>(0xE0 | (codePoint >> 12));
            result += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (codePoint & 0x3F));
        } else if (codePoint <= 0x10FFFF) {
            // 4-byte sequence
            result += static_cast<char>(0xF0 | (codePoint >> 18));
            result += static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
            result += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (codePoint & 0x3F));
        } else {
            // Invalid code point
            result += "\xEF\xBF\xBD"; // UTF-8 encoded replacement character (U+FFFD)
        }
    }
    
    return result;
#else
    return "";
#endif
}

void PDFParsingAdapter::extractMetadata(void* document, ParsedDocument& result) const {
#ifdef LEAFRA_HAS_PDFIUM
    FPDF_DOCUMENT fpdfDoc = static_cast<FPDF_DOCUMENT>(document);
    
    // Helper lambda to extract string metadata
    auto extractStringMetadata = [&](const char* tag) -> std::string {
        unsigned long length = FPDF_GetMetaText(fpdfDoc, tag, nullptr, 0);
        if (length <= 2) return "";
        
        std::vector<unsigned short> buffer(length / 2);
        FPDF_GetMetaText(fpdfDoc, tag, buffer.data(), length);
        
        // Convert UTF-16 to UTF-8 (simple conversion)
        std::string result;
        for (size_t i = 0; i < buffer.size() - 1; ++i) {
            if (buffer[i] < 128) {
                result += static_cast<char>(buffer[i]);
            }
        }
        return result;
    };
    
    // Extract common metadata
    result.title = extractStringMetadata("Title");
    result.author = extractStringMetadata("Author");
    result.metadata["Creator"] = extractStringMetadata("Creator");
    result.metadata["Producer"] = extractStringMetadata("Producer");
    result.metadata["Subject"] = extractStringMetadata("Subject");
    result.metadata["Keywords"] = extractStringMetadata("Keywords");
    result.metadata["CreationDate"] = extractStringMetadata("CreationDate");
    result.metadata["ModDate"] = extractStringMetadata("ModDate");
    
    LEAFRA_DEBUG() << "Extracted metadata - Title: " << result.title << ", Author: " << result.author;
#endif
}

} // namespace leafra 