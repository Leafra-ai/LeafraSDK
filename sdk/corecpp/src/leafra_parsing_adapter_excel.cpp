#include "leafra/leafra_parsing.h"
#include "leafra/logger.h"

#include <algorithm>
#include <cctype>

namespace leafra {

// ==============================================================================
// ExcelParsingAdapter Implementation (Placeholder)
// ==============================================================================

bool ExcelParsingAdapter::canHandle(const std::string& extension) const {
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == "xlsx" || ext == "xls" || ext == "csv";
}

std::vector<std::string> ExcelParsingAdapter::getSupportedExtensions() const {
    return {"xlsx", "xls", "csv"};
}

std::string ExcelParsingAdapter::getName() const {
    return "ExcelParsingAdapter";
}

ParsedDocument ExcelParsingAdapter::parse(const std::string& filePath) const {
    ParsedDocument result;
    result.filePath = filePath;
    result.fileType = "Excel";
    result.errorMessage = "Excel parsing not yet implemented";
    
    LEAFRA_WARNING() << "Excel parsing not yet implemented for: " << filePath;
    
    return result;
}

} // namespace leafra 