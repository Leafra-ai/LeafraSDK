#include "leafra/leafra_core.h"
#include "leafra/data_processor.h"
#include "leafra/math_utils.h"
#include "leafra/platform_utils.h"
#include "leafra/logger.h"
#include <iostream>
#include <sstream>

#ifdef LEAFRA_HAS_PDFIUM
#include "fpdfview.h"
#include "fpdf_text.h"
#include "fpdf_doc.h"
#endif

namespace leafra {

// Private implementation class (PIMPL pattern)
class LeafraCore::Impl {
public:
    Config config_;
    bool initialized_;
    callback_t event_callback_;
    std::unique_ptr<DataProcessor> data_processor_;
    std::unique_ptr<MathUtils> math_utils_;
    
    Impl() : initialized_(false), event_callback_(nullptr) {
        data_processor_ = std::make_unique<DataProcessor>();
        math_utils_ = std::make_unique<MathUtils>();
    }
    
    void send_event(const std::string& message) {
        if (event_callback_) {
            event_callback_(message);
        }
    }
};

LeafraCore::LeafraCore() : pImpl(std::make_unique<Impl>()) {
}

LeafraCore::~LeafraCore() = default;

ResultCode LeafraCore::initialize(const Config& config) {
    if (pImpl->initialized_) {
        return ResultCode::SUCCESS; // Already initialized
    }
    
    try {
        pImpl->config_ = config;
        
        // Initialize logging system
        Logger& logger = Logger::getInstance();
        if (config.debug_mode) {
            logger.setLogLevel(LogLevel::LEAFRA_DEBUG);
            LEAFRA_INFO() << "Debug logging enabled";
        } else {
            logger.setLogLevel(LogLevel::LEAFRA_INFO);
        }
        
        LEAFRA_INFO() << "Initializing LeafraSDK v" << get_version();
        LEAFRA_DEBUG() << "Config - Name: " << config.name << ", Threads: " << config.max_threads << ", Buffer: " << config.buffer_size;
        
        // Initialize data processor
        if (pImpl->data_processor_) {
            ResultCode result = pImpl->data_processor_->initialize();
            if (result != ResultCode::SUCCESS) {
                LEAFRA_ERROR() << "Failed to initialize data processor";
                return result;
            }
            LEAFRA_DEBUG() << "Data processor initialized successfully";
        }
        
        pImpl->initialized_ = true;
        LEAFRA_INFO() << "LeafraSDK initialized successfully";
        
#ifdef LEAFRA_HAS_PDFIUM
        LEAFRA_INFO() << "✅ PDFium integration: ENABLED";
#else
        LEAFRA_WARNING() << "⚠️  PDFium integration: DISABLED (library not found)";
#endif
        
        pImpl->send_event("LeafraSDK initialized successfully");
        
        return ResultCode::SUCCESS;
    } catch (const std::exception& e) {
        LEAFRA_ERROR() << "Initialization failed: " << e.what();
        pImpl->send_event("Initialization failed: " + std::string(e.what()));
        return ResultCode::ERROR_INITIALIZATION_FAILED;
    }
}

ResultCode LeafraCore::shutdown() {
    if (!pImpl->initialized_) {
        return ResultCode::SUCCESS; // Already shutdown
    }
    
    try {
        pImpl->initialized_ = false;
        pImpl->send_event("LeafraSDK shutdown completed");
        return ResultCode::SUCCESS;
    } catch (const std::exception& e) {
        pImpl->send_event("Shutdown failed: " + std::string(e.what()));
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

bool LeafraCore::is_initialized() const {
    return pImpl->initialized_;
}

const Config& LeafraCore::get_config() const {
    return pImpl->config_;
}

ResultCode LeafraCore::process_data(const data_buffer_t& input, data_buffer_t& output) {
    if (!pImpl->initialized_) {
        return ResultCode::ERROR_INITIALIZATION_FAILED;
    }
    
    if (!pImpl->data_processor_) {
        return ResultCode::ERROR_NOT_IMPLEMENTED;
    }
    
    try {
        return pImpl->data_processor_->process(input, output);
    } catch (const std::exception& e) {
        pImpl->send_event("Data processing failed: " + std::string(e.what()));
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

ResultCode LeafraCore::process_user_files(const std::vector<std::string>& file_paths) {
    LEAFRA_INFO() << "Processing " << file_paths.size() << " user files";
    pImpl->send_event("Processing " + std::to_string(file_paths.size()) + " user files");
    
    for (const auto& file_path : file_paths) {
        LEAFRA_DEBUG() << "Processing file: " << file_path;
        pImpl->send_event("Processing file: " + file_path);
        
        // Validate file exists and is PDF
        if (file_path.substr(file_path.find_last_of(".") + 1) != "pdf") {
            LEAFRA_WARNING() << "Skipping non-PDF file: " << file_path;
            pImpl->send_event("Skipping non-PDF file: " + file_path);
            continue;
        }
        
#ifdef LEAFRA_HAS_PDFIUM
        LEAFRA_DEBUG() << "Initializing PDFium for file processing";
        
        // Initialize PDFium (call once)
        static bool pdfium_initialized = false;
        if (!pdfium_initialized) {
            FPDF_InitLibrary();
            pdfium_initialized = true;
            LEAFRA_INFO() << "PDFium library initialized";
        }
        
        // Load PDF document
        FPDF_DOCUMENT document = FPDF_LoadDocument(file_path.c_str(), nullptr);
        if (!document) {
            LEAFRA_ERROR() << "Failed to load PDF: " << file_path;
            pImpl->send_event("Failed to load PDF: " + file_path);
            continue;
        }
        
        // Get page count
        int page_count = FPDF_GetPageCount(document);
        LEAFRA_INFO() << "PDF has " << page_count << " pages: " << file_path;
        pImpl->send_event("PDF has " + std::to_string(page_count) + " pages");
        
        // Process each page
        for (int i = 0; i < page_count; ++i) {
            FPDF_PAGE page = FPDF_LoadPage(document, i);
            if (!page) {
                LEAFRA_WARNING() << "Failed to load page " << (i + 1) << " from " << file_path;
                continue;
            }
            
            // Extract text from page
            FPDF_TEXTPAGE text_page = FPDFText_LoadPage(page);
            if (text_page) {
                int char_count = FPDFText_CountChars(text_page);
                LEAFRA_DEBUG() << "Page " << (i + 1) << " has " << char_count << " characters";
                pImpl->send_event("Page " + std::to_string(i + 1) + " has " + std::to_string(char_count) + " characters");
                
                // Get text content (simplified example)
                if (char_count > 0) {
                    std::vector<unsigned short> buffer(char_count + 1);
                    FPDFText_GetText(text_page, 0, char_count, buffer.data());
                    // Convert and process text as needed
                    LEAFRA_DEBUG() << "Extracted text from page " << (i + 1);
                    pImpl->send_event("Extracted text from page " + std::to_string(i + 1));
                }
                
                FPDFText_ClosePage(text_page);
            }
            
            FPDF_ClosePage(page);
        }
        
        FPDF_CloseDocument(document);
        LEAFRA_INFO() << "Successfully processed PDF: " << file_path;
        pImpl->send_event("Successfully processed PDF: " + file_path);
#else
        LEAFRA_WARNING() << "PDFium not available - simulating processing: " << file_path;
        pImpl->send_event("PDFium not available - simulating processing: " + file_path);
#endif
    }
    
    LEAFRA_INFO() << "File processing completed successfully";
    pImpl->send_event("File processing completed");
    return ResultCode::SUCCESS;
}

void LeafraCore::set_event_callback(callback_t callback) {
    pImpl->event_callback_ = callback;
}

std::string LeafraCore::get_version() {
    std::ostringstream oss;
    oss << LEAFRA_VERSION_MAJOR << "." << LEAFRA_VERSION_MINOR << "." << LEAFRA_VERSION_PATCH;
    return oss.str();
}

std::string LeafraCore::get_platform() {
    return PlatformUtils::get_platform_name();
}

shared_ptr<LeafraCore> LeafraCore::create() {
    return std::make_shared<LeafraCore>();
}

} // namespace leafra 