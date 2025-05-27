#include "leafra/leafra_parsing.h"
#include "leafra/logger.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <cctype>

namespace leafra {

// ==============================================================================
// TextParsingAdapter Implementation
// ==============================================================================

bool TextParsingAdapter::canHandle(const std::string& extension) const {
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == "txt" || ext == "log" || ext == "md" || ext == "readme";
}

std::vector<std::string> TextParsingAdapter::getSupportedExtensions() const {
    return {"txt", "log", "md", "readme"};
}

std::string TextParsingAdapter::getName() const {
    return "TextParsingAdapter";
}

ParsedDocument TextParsingAdapter::parse(const std::string& filePath) const {
    ParsedDocument result;
    result.filePath = filePath;
    result.fileType = "Text";
    
    LEAFRA_INFO() << "Parsing text file: " << filePath;
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        result.errorMessage = "Failed to open text file: " + filePath;
        LEAFRA_ERROR() << result.errorMessage;
        return result;
    }
    
    // Read the entire file
    std::ostringstream content;
    content << file.rdbuf();
    file.close();
    
    std::string fileContent = content.str();
    
    // For text files, treat the entire content as one "page"
    result.pages.push_back(fileContent);
    
    // Extract basic metadata
    std::filesystem::path path(filePath);
    result.title = path.filename().string();
    result.metadata["FileName"] = path.filename().string();
    result.metadata["FileSize"] = std::to_string(fileContent.length());
    result.metadata["LineCount"] = std::to_string(std::count(fileContent.begin(), fileContent.end(), '\n') + 1);
    
    result.isValid = true;
    
    LEAFRA_INFO() << "Successfully parsed text file with " << fileContent.length() << " characters";
    
    return result;
}

} // namespace leafra 