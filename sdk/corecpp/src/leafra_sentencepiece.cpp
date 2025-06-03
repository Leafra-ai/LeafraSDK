#include "leafra/leafra_sentencepiece.h"
#include "leafra/logger.h"

#ifdef LEAFRA_HAS_SENTENCEPIECE
    #include <sentencepiece_processor.h>
    #include <sentencepiece_trainer.h>
#endif

#include <fstream>
#include <sstream>
#include <algorithm>

namespace leafra {

#ifdef LEAFRA_HAS_SENTENCEPIECE

// Implementation class using PIMPL pattern
class SentencePieceTokenizer::Impl {
public:
    sentencepiece::SentencePieceProcessor processor;
    std::string last_error;
    bool loaded = false;

    void set_error(const std::string& error) {
        last_error = error;
        LEAFRA_ERROR() << "SentencePiece error: " << error;
    }

    void clear_error() {
        last_error.clear();
    }
};

#else

// Stub implementation when SentencePiece is not available
class SentencePieceTokenizer::Impl {
public:
    std::string last_error = "SentencePiece not available - compiled without LEAFRA_HAS_SENTENCEPIECE";
    bool loaded = false;
    
    void set_error(const std::string& error) {
        last_error = error;
        LEAFRA_ERROR() << "SentencePiece error: " << error;
    }

    void clear_error() {
        last_error.clear();
    }
};

#endif

// Constructor
SentencePieceTokenizer::SentencePieceTokenizer() : pImpl(std::make_unique<Impl>()) {
    LEAFRA_DEBUG() << "SentencePieceTokenizer created";
}

// Destructor
SentencePieceTokenizer::~SentencePieceTokenizer() = default;

// Move constructor
SentencePieceTokenizer::SentencePieceTokenizer(SentencePieceTokenizer&&) noexcept = default;

// Move assignment
SentencePieceTokenizer& SentencePieceTokenizer::operator=(SentencePieceTokenizer&&) noexcept = default;

bool SentencePieceTokenizer::load_model(const std::string& model_path) {
#ifdef LEAFRA_HAS_SENTENCEPIECE
    LEAFRA_INFO() << "Loading SentencePiece model from: " << model_path;
    
    pImpl->clear_error();
    
    const auto status = pImpl->processor.Load(model_path);
    if (!status.ok()) {
        pImpl->set_error("Failed to load model: " + status.ToString());
        pImpl->loaded = false;
        return false;
    }
    
    pImpl->loaded = true;
    LEAFRA_INFO() << "SentencePiece model loaded successfully";
    LEAFRA_INFO() << "Vocabulary size: " << pImpl->processor.GetPieceSize();
    
    return true;
#else
    pImpl->set_error("SentencePiece not available");
    return false;
#endif
}

bool SentencePieceTokenizer::load_model_from_memory(const std::vector<uint8_t>& model_data) {
#ifdef LEAFRA_HAS_SENTENCEPIECE
    LEAFRA_INFO() << "Loading SentencePiece model from memory (" << model_data.size() << " bytes)";
    
    pImpl->clear_error();
    
    const std::string model_string(model_data.begin(), model_data.end());
    const auto status = pImpl->processor.LoadFromSerializedProto(model_string);
    if (!status.ok()) {
        pImpl->set_error("Failed to load model from memory: " + status.ToString());
        pImpl->loaded = false;
        return false;
    }
    
    pImpl->loaded = true;
    LEAFRA_INFO() << "SentencePiece model loaded from memory successfully";
    LEAFRA_INFO() << "Vocabulary size: " << pImpl->processor.GetPieceSize();
    
    return true;
#else
    pImpl->set_error("SentencePiece not available");
    return false;
#endif
}

bool SentencePieceTokenizer::is_loaded() const {
    return pImpl->loaded;
}

int SentencePieceTokenizer::get_vocab_size() const {
#ifdef LEAFRA_HAS_SENTENCEPIECE
    if (!pImpl->loaded) {
        return 0;
    }
    return pImpl->processor.GetPieceSize();
#else
    return 0;
#endif
}

std::vector<std::string> SentencePieceTokenizer::encode(const std::string& text, const TokenizeOptions& options) const {
#ifdef LEAFRA_HAS_SENTENCEPIECE
    if (!pImpl->loaded) {
        pImpl->set_error("No model loaded");
        return {};
    }
    
    std::vector<std::string> pieces;
    
    if (options.enable_sampling) {
        const auto status = pImpl->processor.SampleEncode(text, options.nbest_size, options.alpha, &pieces);
        if (!status.ok()) {
            pImpl->set_error("Failed to sample encode: " + status.ToString());
            return {};
        }
    } else {
        const auto status = pImpl->processor.Encode(text, &pieces);
        if (!status.ok()) {
            pImpl->set_error("Failed to encode: " + status.ToString());
            return {};
        }
    }
    
    // Apply additional options
    if (options.add_bos) {
        pieces.insert(pieces.begin(), pImpl->processor.IdToPiece(pImpl->processor.bos_id()));
    }
    if (options.add_eos) {
        pieces.push_back(pImpl->processor.IdToPiece(pImpl->processor.eos_id()));
    }
    if (options.reverse) {
        std::reverse(pieces.begin(), pieces.end());
    }
    
    //LEAFRA_DEBUG() << "Encoded text '" << text << "' into " << pieces.size() << " pieces";
    return pieces;
#else
    pImpl->set_error("SentencePiece not available");
    return {};
#endif
}

std::vector<int> SentencePieceTokenizer::encode_as_ids(const std::string& text, const TokenizeOptions& options) const {
#ifdef LEAFRA_HAS_SENTENCEPIECE
    if (!pImpl->loaded) {
        pImpl->set_error("No model loaded");
        return {};
    }
    
    std::vector<int> ids;
    
    if (options.enable_sampling) {
        const auto status = pImpl->processor.SampleEncode(text, options.nbest_size, options.alpha, &ids);
        if (!status.ok()) {
            pImpl->set_error("Failed to sample encode as IDs: " + status.ToString());
            return {};
        }
    } else {
        const auto status = pImpl->processor.Encode(text, &ids);
        if (!status.ok()) {
            pImpl->set_error("Failed to encode as IDs: " + status.ToString());
            return {};
        }
    }
    
    // Apply additional options
    if (options.add_bos) {
        ids.insert(ids.begin(), pImpl->processor.bos_id());
    }
    if (options.add_eos) {
        ids.push_back(pImpl->processor.eos_id());
    }
    if (options.reverse) {
        std::reverse(ids.begin(), ids.end());
    }
    
    //LEAFRA_DEBUG() << "Encoded text '" << text << "' into " << ids.size() << " token IDs";
    return ids;
#else
    pImpl->set_error("SentencePiece not available");
    return {};
#endif
}

std::string SentencePieceTokenizer::decode(const std::vector<std::string>& pieces) const {
#ifdef LEAFRA_HAS_SENTENCEPIECE
    if (!pImpl->loaded) {
        pImpl->set_error("No model loaded");
        return "";
    }
    
    std::string text;
    const auto status = pImpl->processor.Decode(pieces, &text);
    if (!status.ok()) {
        pImpl->set_error("Failed to decode pieces: " + status.ToString());
        return "";
    }
    
    LEAFRA_DEBUG() << "Decoded " << pieces.size() << " pieces into text: '" << text << "'";
    return text;
#else
    pImpl->set_error("SentencePiece not available");
    return "";
#endif
}

std::string SentencePieceTokenizer::decode(const std::vector<int>& ids) const {
#ifdef LEAFRA_HAS_SENTENCEPIECE
    if (!pImpl->loaded) {
        pImpl->set_error("No model loaded");
        return "";
    }
    
    std::string text;
    const auto status = pImpl->processor.Decode(ids, &text);
    if (!status.ok()) {
        pImpl->set_error("Failed to decode IDs: " + status.ToString());
        return "";
    }
    
    LEAFRA_DEBUG() << "Decoded " << ids.size() << " token IDs into text: '" << text << "'";
    return text;
#else
    pImpl->set_error("SentencePiece not available");
    return "";
#endif
}

int SentencePieceTokenizer::piece_to_id(const std::string& piece) const {
#ifdef LEAFRA_HAS_SENTENCEPIECE
    if (!pImpl->loaded) {
        return unk_id();
    }
    return pImpl->processor.PieceToId(piece);
#else
    return -1;
#endif
}

std::string SentencePieceTokenizer::id_to_piece(int id) const {
#ifdef LEAFRA_HAS_SENTENCEPIECE
    if (!pImpl->loaded) {
        return "";
    }
    return pImpl->processor.IdToPiece(id);
#else
    return "";
#endif
}

int SentencePieceTokenizer::unk_id() const {
#ifdef LEAFRA_HAS_SENTENCEPIECE
    if (!pImpl->loaded) {
        return -1;
    }
    return pImpl->processor.unk_id();
#else
    return -1;
#endif
}

int SentencePieceTokenizer::bos_id() const {
#ifdef LEAFRA_HAS_SENTENCEPIECE
    if (!pImpl->loaded) {
        return -1;
    }
    return pImpl->processor.bos_id();
#else
    return -1;
#endif
}

int SentencePieceTokenizer::eos_id() const {
#ifdef LEAFRA_HAS_SENTENCEPIECE
    if (!pImpl->loaded) {
        return -1;
    }
    return pImpl->processor.eos_id();
#else
    return -1;
#endif
}

int SentencePieceTokenizer::pad_id() const {
#ifdef LEAFRA_HAS_SENTENCEPIECE
    if (!pImpl->loaded) {
        return -1;
    }
    return pImpl->processor.pad_id();
#else
    return -1;
#endif
}

bool SentencePieceTokenizer::train_model(const std::vector<std::string>& input_files,
                                        const std::string& model_prefix,
                                        const TrainOptions& options) {
#ifdef LEAFRA_HAS_SENTENCEPIECE
    LEAFRA_INFO() << "Training SentencePiece model with prefix: " << model_prefix;
    
    // Build training arguments
    std::ostringstream args;
    
    // Input files
    args << "--input=";
    for (size_t i = 0; i < input_files.size(); ++i) {
        if (i > 0) args << ",";
        args << input_files[i];
    }
    
    // Basic options
    args << " --model_prefix=" << model_prefix;
    args << " --model_type=" << options.model_type;
    args << " --vocab_size=" << options.vocab_size;
    args << " --character_coverage=" << options.character_coverage;
    args << " --num_threads=" << options.num_threads;
    args << " --max_sentence_length=" << options.max_sentence_length;
    
    // Boolean options
    if (options.shuffle_input_sentence) args << " --shuffle_input_sentence=true";
    if (options.split_by_unicode_script) args << " --split_by_unicode_script=true";
    if (options.split_by_number) args << " --split_by_number=true";
    if (options.split_by_whitespace) args << " --split_by_whitespace=true";
    if (options.byte_fallback) args << " --byte_fallback=true";
    
    // Optional string parameters
    if (!options.input_sentence_size.empty()) {
        args << " --input_sentence_size=" << options.input_sentence_size;
    }
    if (!options.control_symbols.empty()) {
        args << " --control_symbols=" << options.control_symbols;
    }
    if (!options.user_defined_symbols.empty()) {
        args << " --user_defined_symbols=" << options.user_defined_symbols;
    }
    
    const std::string train_args = args.str();
    LEAFRA_INFO() << "Training arguments: " << train_args;
    
    const auto status = sentencepiece::SentencePieceTrainer::Train(train_args);
    if (!status.ok()) {
        LEAFRA_ERROR() << "Failed to train model: " << status.ToString();
        return false;
    }
    
    LEAFRA_INFO() << "SentencePiece model training completed successfully";
    return true;
#else
    LEAFRA_ERROR() << "SentencePiece not available - cannot train model";
    return false;
#endif
}

std::string SentencePieceTokenizer::get_model_info() const {
#ifdef LEAFRA_HAS_SENTENCEPIECE
    if (!pImpl->loaded) {
        return "No model loaded";
    }
    
    std::ostringstream info;
    info << "SentencePiece Model Information:\n";
    info << "  Vocabulary size: " << pImpl->processor.GetPieceSize() << "\n";
    info << "  UNK ID: " << pImpl->processor.unk_id() << "\n";
    info << "  BOS ID: " << pImpl->processor.bos_id() << "\n";
    info << "  EOS ID: " << pImpl->processor.eos_id() << "\n";
    info << "  PAD ID: " << pImpl->processor.pad_id() << "\n";
    
    return info.str();
#else
    return "SentencePiece not available";
#endif
}

std::string SentencePieceTokenizer::get_last_error() const {
    return pImpl->last_error;
}

// Utility functions
namespace sentencepiece_utils {

bool is_available() {
#ifdef LEAFRA_HAS_SENTENCEPIECE
    return true;
#else
    return false;
#endif
}

std::string get_version() {
#ifdef LEAFRA_HAS_SENTENCEPIECE
    // SentencePiece doesn't provide a simple version method in newer versions
    // Return a placeholder version string indicating SentencePiece is available
    return "SentencePiece (available)";
#else
    return "Not available";
#endif
}

std::string normalize_text(const std::string& text) {
#ifdef LEAFRA_HAS_SENTENCEPIECE
    sentencepiece::SentencePieceProcessor processor;
    std::string normalized;
    
    // Use a simple normalization approach
    // In practice, you might want to load a specific model for normalization
    const auto status = processor.Normalize(text, &normalized);
    if (status.ok()) {
        return normalized;
    }
    
    LEAFRA_WARNING() << "Failed to normalize text: " << status.ToString();
    return text; // Return original text if normalization fails
#else
    LEAFRA_WARNING() << "SentencePiece not available - returning original text";
    return text;
#endif
}

std::vector<std::string> quick_tokenize(const std::string& text, const std::string& model_path) {
#ifdef LEAFRA_HAS_SENTENCEPIECE
    sentencepiece::SentencePieceProcessor processor;
    const auto status = processor.Load(model_path);
    if (!status.ok()) {
        LEAFRA_ERROR() << "Failed to load model for quick tokenization: " << status.ToString();
        return {};
    }
    
    std::vector<std::string> pieces;
    const auto encode_status = processor.Encode(text, &pieces);
    if (!encode_status.ok()) {
        LEAFRA_ERROR() << "Failed to tokenize text: " << encode_status.ToString();
        return {};
    }
    
    return pieces;
#else
    LEAFRA_ERROR() << "SentencePiece not available";
    return {};
#endif
}

} // namespace sentencepiece_utils

} // namespace leafra 