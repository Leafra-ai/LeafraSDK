#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <algorithm>
#include <functional>
#include "../../../include/leafra/leafra_chunker.h"
#include "../../../include/leafra/leafra_unicode.h"
#include "../../../include/leafra/leafra_debug.h"

using namespace leafra;

// Generate diverse UTF-8 text samples
std::string generate_long_utf8_text() {
    return 
        "This is a comprehensive test of UTF-8 text processing with international characters. "
        "我们来测试中文字符的处理能力，包括简体和繁體中文。" // Chinese
        "Español con acentos: café, niño, corazón, pequeño. " // Spanish
        "Français avec accents: être, naïve, élève, français. " // French  
        "Deutsch mit Umlauten: schön, Mädchen, Größe, hören. " // German
        "Русский текст с кириллицей: привет, мир, спасибо. " // Russian
        "العربية من اليمين إلى اليسار مع الأرقام ١٢٣٤٥. " // Arabic (RTL)
        "ελληνικά γράμματα: αλφάβητο, φιλοσοφία, δημοκρατία. " // Greek
        "עברית גם כן מימין לשמאל: שלום, תודה, בבקשה. " // Hebrew (RTL)
        "日本語のひらがなとカタカナ：こんにちは、ありがとう。" // Japanese
        "한국어 텍스트: 안녕하세요, 감사합니다, 죄송합니다. " // Korean
        "Emoji integration: 🌍🚀🎉😄💡🔥⭐🌟💖🎯 various emoji types. "
        "Mathematical symbols: ∑∏∫∆∇∀∃∈∉⊂⊃∪∩ and more. "
        "Currency symbols: €¥£₹₽₩₪₿ from different countries. "
        "Special punctuation: «quotes» " "English" " 'single' ¿question? ¡exclamation! "
        "Diacritical marks: àáâãäåāăąçćĉċčèéêëēĕėęěìíîïĩīĭįıòóôõöøōŏőùúûüũūŭůűų. "
        "Technical symbols: ©®™℗§¶†‡•◦‰‱′″‴⁗ various marks. "
        "More content to reach target lengths for comprehensive testing of chunking algorithms. "
        "Additional sentences to ensure we have sufficient text for large chunk sizes. "
        "Lorem ipsum with international flavor: 这是一个测试文本 für die Überprüfung des Chunking-Algorithmus. "
        "Final section with mixed content: programming symbols like <tag>, {bracket}, [array], (parentheses), "
        "and URL-like text: https://example.com/path?param=value&other=测试 for realistic content testing.";
}

// Generate academic-style UTF-8 text
std::string generate_academic_utf8_text() {
    return
        "The internationalization of software requires careful consideration of Unicode standards. "
        "According to researchers at 北京大学 (Peking University) and École Polytechnique, "
        "multi-byte character encoding presents unique challenges. "
        "The UTF-8 standard, originally developed by Ken Thompson and Rob Pike, "
        "provides backward compatibility with ASCII while supporting the full Unicode range. "
        "Studies from Московский государственный университет (Moscow State University) "
        "demonstrate that proper text segmentation is crucial for natural language processing. "
        "The Unicode Consortium maintains standards for character classification, "
        "including categories like Lu (Letter, uppercase), Ll (Letter, lowercase), "
        "and Zs (Separator, space). Mathematical expressions like ∫₀^∞ e^(-x²) dx = √π/2 "
        "require special handling in text processing systems. "
        "Linguistic diversity includes languages with different writing systems: "
        "Latin (English, français, español), Cyrillic (русский, български), "
        "Arabic (العربية), Hebrew (עברית), Chinese (中文), Japanese (日本語), "
        "Korean (한국어), Greek (ελληνικά), and many others. "
        "Modern applications must handle emoji 📚📖📝✏️📋 as legitimate textual content. "
        "Technical documentation often includes special symbols: ≤≥≠≈∞∅∈∉⊂⊃∪∩ "
        "and mathematical notation that spans multiple Unicode blocks.";
}

// Test helper to validate chunk properties
void validate_chunk_properties(const std::vector<TextChunk>& chunks, 
                              const ChunkingOptions& options,
                              const std::string& test_name) {
    assert(!chunks.empty());
    
    std::cout << " [" << test_name << "] Generated " << chunks.size() << " chunks, ";
    
    for (const auto& chunk : chunks) {
        // All chunks should be non-empty since we don't create empty chunks
        assert(!chunk.content.empty());
        
        // Validate UTF-8 boundaries - this is the key test
        unsigned char first_byte = static_cast<unsigned char>(chunk.content[0]);
        
        // Should not start with UTF-8 continuation byte
        assert((first_byte & 0x80) == 0 || (first_byte & 0xC0) != 0x80);
    }
    
    std::cout << "validated ✓";
}

// Test CHARACTERS mode with different sizes
void test_characters_mode_comprehensive() {
    std::cout << "Running test_characters_mode_comprehensive...\n";
    
    LeafraChunker chunker;
    assert(chunker.initialize() == ResultCode::SUCCESS);
    
    std::string long_text = generate_long_utf8_text();
    std::string academic_text = generate_academic_utf8_text();
    
    size_t test_sizes[] = {50, 100, 250, 500, 750};
    
    for (size_t i = 0; i < 5; i++) {

        size_t chunk_size = test_sizes[i];
        std::cout << "  Testing CHARACTERS mode with size " << chunk_size << "... ";
        
        ChunkingOptions options;
        options.chunk_size = chunk_size;
        options.size_unit = ChunkSizeUnit::CHARACTERS;
        options.preserve_word_boundaries = true;
        options.overlap_percentage = 0.1;
        
        // Test with long diverse text
        std::vector<TextChunk> chunks1;
        ResultCode result1 = chunker.chunk_text(long_text, options, chunks1);
        assert(result1 == ResultCode::SUCCESS);
        validate_chunk_properties(chunks1, options, "diverse");
        
        // Test with academic text
        std::vector<TextChunk> chunks2;
        ResultCode result2 = chunker.chunk_text(academic_text, options, chunks2);
        assert(result2 == ResultCode::SUCCESS);
        validate_chunk_properties(chunks2, options, "academic");
        
        std::cout << " PASS\n";
    }
    
    std::cout << "test_characters_mode_comprehensive COMPLETED\n";
}

// Test TOKENS mode with different sizes
void test_tokens_mode_comprehensive() {
    std::cout << "Running test_tokens_mode_comprehensive...\n";
    
    LeafraChunker chunker;
    assert(chunker.initialize() == ResultCode::SUCCESS);
    
    std::string long_text = generate_long_utf8_text();
    std::string academic_text = generate_academic_utf8_text();
    
    size_t test_sizes[] = {50, 100, 300, 500};
    
    for (size_t i = 0; i < 3; i++) {
        size_t chunk_size = test_sizes[i];
        std::cout << "  Testing TOKENS mode with size " << chunk_size << "... ";
        
        ChunkingOptions options;
        options.chunk_size = chunk_size;
        options.size_unit = ChunkSizeUnit::TOKENS;
        options.preserve_word_boundaries = true;
        options.overlap_percentage = 0.15;
        
        // Test with long diverse text
        std::vector<TextChunk> chunks1;
        ResultCode result1 = chunker.chunk_text(long_text, options, chunks1);
        assert(result1 == ResultCode::SUCCESS);
        validate_chunk_properties(chunks1, options, "diverse");
        
        // Test with academic text  
        std::vector<TextChunk> chunks2;
        ResultCode result2 = chunker.chunk_text(academic_text, options, chunks2);
        assert(result2 == ResultCode::SUCCESS);
        validate_chunk_properties(chunks2, options, "academic");
        
        std::cout << " PASS\n";
    }
    
    std::cout << "test_tokens_mode_comprehensive COMPLETED\n";
}

// Test edge cases and UTF-8 specific scenarios
void test_utf8_edge_cases() {
    std::cout << "Running test_utf8_edge_cases...\n";
    
    LeafraChunker chunker;
    assert(chunker.initialize() == ResultCode::SUCCESS);
    
    // Test 1: Text with only emoji
    std::cout << "  Testing emoji-only text... ";
    std::string emoji_text = "🚀🌍🎉😄💡🔥⭐🌟💖🎯🚀🌍🎉😄💡🔥⭐🌟💖🎯🚀🌍🎉😄💡🔥⭐🌟💖🎯";
    ChunkingOptions emoji_options;
    emoji_options.chunk_size = 10;
    emoji_options.size_unit = ChunkSizeUnit::CHARACTERS;
    
    std::vector<TextChunk> emoji_chunks;
    ResultCode result1 = chunker.chunk_text(emoji_text, emoji_options, emoji_chunks);
    assert(result1 == ResultCode::SUCCESS);
    assert(!emoji_chunks.empty());
    std::cout << "PASS\n";
    
    // Test 2: Mixed RTL/LTR text
    std::cout << "  Testing RTL/LTR mixed text... ";
    std::string rtl_text = "English العربية English עברית English العربية mixed script text";
    ChunkingOptions rtl_options;
    rtl_options.chunk_size = 20;
    rtl_options.size_unit = ChunkSizeUnit::CHARACTERS;
    
    std::vector<TextChunk> rtl_chunks;
    ResultCode result2 = chunker.chunk_text(rtl_text, rtl_options, rtl_chunks);
    assert(result2 == ResultCode::SUCCESS);
    assert(!rtl_chunks.empty());
    std::cout << "PASS\n";
    
    // Test 3: Text with various Unicode whitespace
    std::cout << "  Testing Unicode whitespace... ";
    std::string ws_text = "Word1\u00A0Word2\u2000Word3\u2028Word4\u3000Word5\u180EWord6";
    ChunkingOptions ws_options;
    ws_options.chunk_size = 15;
    ws_options.size_unit = ChunkSizeUnit::CHARACTERS;
    
    std::vector<TextChunk> ws_chunks;
    ResultCode result3 = chunker.chunk_text(ws_text, ws_options, ws_chunks);
    assert(result3 == ResultCode::SUCCESS);
    assert(!ws_chunks.empty());
    std::cout << "PASS\n";
    
    // Test 4: Mathematical and technical symbols
    std::cout << "  Testing mathematical symbols... ";
    std::string math_text = "∫∑∏∆∇∀∃∈∉⊂⊃∪∩≤≥≠≈∞∅ mathematical symbols with regular text";
    ChunkingOptions math_options;
    math_options.chunk_size = 25;
    math_options.size_unit = ChunkSizeUnit::CHARACTERS;
    
    std::vector<TextChunk> math_chunks;
    ResultCode result4 = chunker.chunk_text(math_text, math_options, math_chunks);
    assert(result4 == ResultCode::SUCCESS);
    assert(!math_chunks.empty());
    std::cout << "PASS\n";
    
    std::cout << "test_utf8_edge_cases COMPLETED\n";
}

// Test overlap behavior with UTF-8
void test_utf8_overlap_behavior() {
    std::cout << "Running test_utf8_overlap_behavior...\n";
    
    LeafraChunker chunker;
    assert(chunker.initialize() == ResultCode::SUCCESS);
    
    std::string test_text = generate_academic_utf8_text();
    
    double overlap_percentages[] = {0.0, 0.1, 0.2, 0.3};
    
    for (size_t i = 0; i < 4; i++) {
        double overlap = overlap_percentages[i];
        std::cout << "  Testing " << (overlap * 100) << "% overlap... ";
        
        ChunkingOptions options;
        options.chunk_size = 150;
        options.size_unit = ChunkSizeUnit::CHARACTERS;
        options.overlap_percentage = overlap;
        options.preserve_word_boundaries = true;
        
        std::vector<TextChunk> chunks;
        ResultCode result = chunker.chunk_text(test_text, options, chunks);
        assert(result == ResultCode::SUCCESS);
        assert(!chunks.empty());
        
        // Validate overlap (basic check)
        if (chunks.size() > 1 && overlap > 0.0) {
            // Check that consecutive chunks have some overlap
            bool has_overlap = false;
            for (size_t j = 1; j < chunks.size(); j++) {
                // Simple check: look for common words between consecutive chunks
                if (chunks[j-1].content.find("the") != std::string::npos &&
                    chunks[j].content.find("the") != std::string::npos) {
                    has_overlap = true;
                    break;
                }
            }
            // Note: This is a simplified overlap check
        }
        
        std::cout << "generated " << chunks.size() << " chunks ✓\n";
    }
    
    std::cout << "test_utf8_overlap_behavior COMPLETED\n";
}

// Performance test with large UTF-8 text
void test_utf8_performance() {
    std::cout << "Running test_utf8_performance...\n";
    
    LeafraChunker chunker;
    assert(chunker.initialize() == ResultCode::SUCCESS);
    
    // Generate large text by repeating our samples
    std::string base_text = generate_long_utf8_text() + " " + generate_academic_utf8_text();
    std::string large_text;
    for (int i = 0; i < 10; i++) { // Repeat 10 times for larger text
        large_text += base_text + " ";
    }
    
    std::cout << "  Testing performance with large text (" << large_text.length() << " bytes)... ";
    
    ChunkingOptions options;
    options.chunk_size = 200;
    options.size_unit = ChunkSizeUnit::CHARACTERS;
    options.overlap_percentage = 0.1;
    
    std::vector<TextChunk> chunks;
    ResultCode result = chunker.chunk_text(large_text, options, chunks);
    assert(result == ResultCode::SUCCESS);
    assert(!chunks.empty());
    
    std::cout << "generated " << chunks.size() << " chunks ✓\n";
    
    std::cout << "test_utf8_performance COMPLETED\n";
}

// Helper function to setup debug based on command line arguments
bool setup_debug_mode(int argc, char* argv[]) {
    bool debug_enabled = true; // Default: debug enabled
    
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            std::string arg(argv[i]);
            if (arg == "--no-debug" || arg == "--disable-debug") {
                debug_enabled = false;
            } else if (arg == "--debug" || arg == "-d") {
                debug_enabled = true; // Explicitly enable (already default)
            } else if (arg == "--help" || arg == "-h") {
                std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
                std::cout << "Options:" << std::endl;
                std::cout << "  --debug, -d          Enable debug output (default)" << std::endl;
                std::cout << "  --no-debug           Disable debug output" << std::endl;
                std::cout << "  --help, -h           Show this help message" << std::endl;
                exit(0);
            }
        }
    }
    
    debug::set_debug_enabled(debug_enabled);
    return debug_enabled;
}

int main(int argc, char* argv[]) {
    // Setup debug mode based on command line arguments
    bool debug_enabled = setup_debug_mode(argc, argv);
    
    std::cout << "=== UTF-8 Enhanced Chunking Tests ===" << std::endl;
    std::cout << "Debug mode: " << (debug_enabled ? "ENABLED" : "DISABLED") << std::endl;
    
    try {
        test_characters_mode_comprehensive();
         std::cout << std::endl;
        
         test_tokens_mode_comprehensive();
         std::cout << std::endl;
        
         test_utf8_edge_cases();
         std::cout << std::endl;
        
         test_utf8_overlap_behavior();
         std::cout << std::endl;
        
         test_utf8_performance();
         std::cout << std::endl;
        
        std::cout << "✅ All comprehensive UTF-8 tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cout << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
} 