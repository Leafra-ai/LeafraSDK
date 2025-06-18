/*
** LeafraSDK SentencePiece Integration
** Cross-platform text tokenization using SentencePiece
*/

#ifndef LEAFRA_SENTENCEPIECE_H
#define LEAFRA_SENTENCEPIECE_H

#ifdef __cplusplus

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace leafra {

// Forward declaration
struct TokenizerConfig;

/**
 * @brief SentencePiece tokenizer wrapper for LeafraSDK
 * 
 * Provides text tokenization and detokenization capabilities using Google's SentencePiece library.
 * Supports BPE (Byte Pair Encoding) and Unigram language models.
 */
class SentencePieceTokenizer {
public:
    /**
     * @brief Tokenization options
     */
    struct TokenizeOptions {
        bool enable_sampling;      ///< Enable subword regularization
        float alpha;               ///< Sampling parameter (0.0 = deterministic)
        int nbest_size;            ///< Number of best segmentations (-1 = all)
        bool add_bos;              ///< Add beginning-of-sentence token
        bool add_eos;              ///< Add end-of-sentence token
        bool reverse;              ///< Reverse the input sequence
        
        // Default constructor
        TokenizeOptions() : enable_sampling(false), alpha(0.1f), nbest_size(-1), 
                           add_bos(true), add_eos(true), reverse(false) {}
    };

    /**
     * @brief Training options for creating new models
     */
    struct TrainOptions {
        std::string model_type;              ///< Model type: unigram, bpe, char, word
        int vocab_size;                      ///< Vocabulary size
        float character_coverage;            ///< Character coverage (0.9995 for CJK, 1.0 for others)
        std::string input_sentence_size;     ///< Max input sentence size
        bool shuffle_input_sentence;         ///< Shuffle input sentences
        int seed_sentencepiece_size;         ///< Seed sentence piece size
        bool shrinking_factor;               ///< Enable shrinking factor
        int num_threads;                     ///< Number of threads for training
        int num_sub_iterations;              ///< Number of EM sub-iterations
        int max_sentence_length;             ///< Maximum sentence length
        bool split_by_unicode_script;        ///< Split by Unicode script
        bool split_by_number;                ///< Split numbers
        bool split_by_whitespace;            ///< Split by whitespace
        std::string control_symbols;         ///< Control symbols
        std::string user_defined_symbols;    ///< User-defined symbols
        bool byte_fallback;                  ///< Enable byte fallback
        std::string unk_surface;             ///< Unknown surface form
        
        // Default constructor
        TrainOptions() : model_type("unigram"), vocab_size(8000), character_coverage(0.9995f),
                        input_sentence_size(""), shuffle_input_sentence(true),
                        seed_sentencepiece_size(1000000), shrinking_factor(true),
                        num_threads(16), num_sub_iterations(2), max_sentence_length(4192),
                        split_by_unicode_script(true), split_by_number(true), 
                        split_by_whitespace(true), control_symbols(""), 
                        user_defined_symbols(""), byte_fallback(false), unk_surface(" \u2047 ") {}
    };

    SentencePieceTokenizer();
    ~SentencePieceTokenizer();

    // Non-copyable but movable
    SentencePieceTokenizer(const SentencePieceTokenizer&) = delete;
    SentencePieceTokenizer& operator=(const SentencePieceTokenizer&) = delete;
    SentencePieceTokenizer(SentencePieceTokenizer&&) noexcept;
    SentencePieceTokenizer& operator=(SentencePieceTokenizer&&) noexcept;

    /**
     * @brief Load a trained SentencePiece model from TokenizerConfig
     * @param config Tokenizer configuration containing model path and options
     * @return true if successful, false otherwise
     */
    bool load_model(const TokenizerConfig& config);


    /**
     * @brief Check if a model is loaded
     * @return true if model is loaded, false otherwise
     */
    bool is_loaded() const;

    /**
     * @brief Get vocabulary size
     * @return Vocabulary size, or 0 if no model loaded
     */
    int get_vocab_size() const;

    /**
     * @brief Tokenize text into pieces (strings)
     * @param text Input text
     * @param options Tokenization options
     * @return Vector of token strings
     */
    std::vector<std::string> encode(const std::string& text, const TokenizeOptions& options = TokenizeOptions()) const;

    /**
     * @brief Tokenize text into IDs
     * @param text Input text
     * @param options Tokenization options
     * @return Vector of token IDs
     */
    std::vector<int> encode_as_ids(const std::string& text, const TokenizeOptions& options = TokenizeOptions()) const;

    /**
     * @brief Detokenize pieces back to text
     * @param pieces Vector of token strings
     * @return Detokenized text
     */
    std::string decode(const std::vector<std::string>& pieces) const;

    /**
     * @brief Detokenize IDs back to text
     * @param ids Vector of token IDs
     * @return Detokenized text
     */
    std::string decode(const std::vector<int>& ids) const;

    /**
     * @brief Convert token string to ID
     * @param piece Token string
     * @return Token ID, or unk_id() if not found
     */
    int piece_to_id(const std::string& piece) const;

    /**
     * @brief Convert token ID to string
     * @param id Token ID
     * @return Token string, or empty if invalid ID
     */
    std::string id_to_piece(int id) const;

    /**
     * @brief Get unknown token ID
     * @return Unknown token ID
     */
    int unk_id() const;

    /**
     * @brief Get beginning-of-sentence token ID
     * @return BOS token ID
     */
    int bos_id() const;

    /**
     * @brief Get end-of-sentence token ID
     * @return EOS token ID
     */
    int eos_id() const;

    /**
     * @brief Get padding token ID
     * @return Padding token ID
     */
    int pad_id() const;

    /**
     * @brief Train a new SentencePiece model
     * @param input_files Vector of input text files
     * @param model_prefix Output model prefix (will create .model and .vocab files)
     * @param options Training options
     * @return true if successful, false otherwise
     */
    static bool train_model(const std::vector<std::string>& input_files,
                           const std::string& model_prefix,
                           const TrainOptions& options = TrainOptions());

    /**
     * @brief Get model information as string
     * @return Model information, or empty string if no model loaded
     */
    std::string get_model_info() const;

    /**
     * @brief Get error message from last operation
     * @return Error message
     */
    std::string get_last_error() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

/**
 * @brief Utility functions for SentencePiece operations
 */
namespace sentencepiece_utils {

/**
 * @brief Check if SentencePiece is available
 * @return true if SentencePiece is compiled in, false otherwise
 */
bool is_available();

/**
 * @brief Get SentencePiece version string
 * @return Version string
 */
std::string get_version();

/**
 * @brief Normalize text using SentencePiece normalization
 * @param text Input text
 * @return Normalized text
 */
std::string normalize_text(const std::string& text);

/**
 * @brief Quick tokenization without loading a model (for testing)
 * @param text Input text
 * @param model_path Path to model file
 * @return Vector of token strings
 */
std::vector<std::string> quick_tokenize(const std::string& text, const std::string& model_path);

} // namespace sentencepiece_utils

} // namespace leafra

#endif // __cplusplus

#endif // LEAFRA_SENTENCEPIECE_H 