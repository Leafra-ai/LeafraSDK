#include "leafra/leafra_parsing.h"
#include "leafra/logger.h"

#include <algorithm>
#include <cctype>

namespace leafra {

// ==============================================================================
// DOCXParsingAdapter Implementation (Placeholder)
// ==============================================================================

bool DOCXParsingAdapter::canHandle(const std::string& extension) const {
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == "docx" || ext == "doc";
}

std::vector<std::string> DOCXParsingAdapter::getSupportedExtensions() const {
    return {"docx", "doc"};
}

std::string DOCXParsingAdapter::getName() const {
    return "DOCXParsingAdapter";
}

ParsedDocument DOCXParsingAdapter::parse(const std::string& filePath) const {
    ParsedDocument result;
    result.filePath = filePath;
    result.fileType = "DOCX";
    result.errorMessage = "DOCX parsing not yet implemented";
    
    LEAFRA_WARNING() << "DOCX parsing not yet implemented for: " << filePath;
    
    return result;
}

} // namespace leafra 