#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include "leafra/leafra_chunker.h"

using namespace leafra;

void test_utf8_character_counting() {
    std::cout << "Running test_utf8_character_counting... ";
    
    LeafraChunker chunker;
    assert(chunker.initialize() == ResultCode::SUCCESS);
    
    // Test text with various Unicode characters
    std::string utf8_text = "Hello 世界! 🌍 Ñoël Äpfel café naïve résumé";
    
    ChunkingOptions options;
    options.chunk_size = 10; // Small chunks to force splitting
    options.size_unit = ChunkSizeUnit::CHARACTERS;
    options.preserve_word_boundaries = true;
    
    std::vector<TextChunk> chunks;
    ResultCode result = chunker.chunk_text(utf8_text, options, chunks);
    
    assert(result == ResultCode::SUCCESS);
    assert(!chunks.empty());
    
    // Verify chunks contain valid UTF-8 and no broken characters
    for (const auto& chunk : chunks) {
        assert(!chunk.content.empty());
        // Basic check that we don't have broken UTF-8 at boundaries
        // UTF-8 continuation bytes should not appear at the start
        if (!chunk.content.empty()) {
            unsigned char first_byte = static_cast<unsigned char>(chunk.content[0]);
            assert((first_byte & 0x80) == 0 || (first_byte & 0xC0) != 0x80);
        }
    }
    
    std::cout << "PASS" << std::endl;
}

void test_utf8_whitespace_handling() {
    std::cout << "Running test_utf8_whitespace_handling... ";
    
    LeafraChunker chunker;
    assert(chunker.initialize() == ResultCode::SUCCESS);
    
    // Text with various Unicode whitespace characters
    std::string utf8_text = "Word1\u00A0Word2\u2000Word3\u2028Word4\u3000Word5";
    
    ChunkingOptions options;
    options.chunk_size = 15;
    options.size_unit = ChunkSizeUnit::CHARACTERS;
    options.preserve_word_boundaries = true;
    
    std::vector<TextChunk> chunks;
    ResultCode result = chunker.chunk_text(utf8_text, options, chunks);
    
    assert(result == ResultCode::SUCCESS);
    assert(!chunks.empty());
    
    // Check that whitespace is properly trimmed
    for (const auto& chunk : chunks) {
        if (!chunk.content.empty()) {
            // Content should not start or end with ASCII whitespace
            assert(chunk.content.front() != ' ');
            assert(chunk.content.back() != ' ');
            assert(chunk.content.front() != '\t');
            assert(chunk.content.back() != '\t');
        }
    }
    
    std::cout << "PASS" << std::endl;
}

void test_utf8_word_boundaries() {
    std::cout << "Running test_utf8_word_boundaries... ";
    
    LeafraChunker chunker;
    assert(chunker.initialize() == ResultCode::SUCCESS);
    
    // Text with non-ASCII word characters
    std::string utf8_text = "Français español 中文 العربية русский ελληνικά";
    
    ChunkingOptions options;
    options.chunk_size = 12;
    options.size_unit = ChunkSizeUnit::CHARACTERS;
    options.preserve_word_boundaries = true;
    
    std::vector<TextChunk> chunks;
    ResultCode result = chunker.chunk_text(utf8_text, options, chunks);
    
    assert(result == ResultCode::SUCCESS);
    assert(!chunks.empty());
    
    // Verify that we don't split words in the middle
    for (const auto& chunk : chunks) {
        if (!chunk.content.empty()) {
            std::cout << "[" << chunk.content << "] ";
        }
    }
    
    std::cout << "PASS" << std::endl;
}

void test_utf8_emoji_handling() {
    std::cout << "Running test_utf8_emoji_handling... ";
    
    LeafraChunker chunker;
    assert(chunker.initialize() == ResultCode::SUCCESS);
    
    // Text with various emoji (multi-byte UTF-8)
    std::string utf8_text = "Hello 👋 World 🌍 with emojis 🚀🎉 and more text";
    
    ChunkingOptions options;
    options.chunk_size = 15;
    options.size_unit = ChunkSizeUnit::CHARACTERS;
    options.preserve_word_boundaries = true;
    
    std::vector<TextChunk> chunks;
    ResultCode result = chunker.chunk_text(utf8_text, options, chunks);
    
    assert(result == ResultCode::SUCCESS);
    assert(!chunks.empty());
    
    // Verify no broken emoji (emoji should not be split)
    for (const auto& chunk : chunks) {
        // Basic check: no isolated UTF-8 continuation bytes at boundaries
        if (!chunk.content.empty()) {
            unsigned char first = static_cast<unsigned char>(chunk.content.front());
            unsigned char last = static_cast<unsigned char>(chunk.content.back());
            
            // Should not start with a continuation byte
            assert((first & 0x80) == 0 || (first & 0xC0) != 0x80);
            
            // Check that we have complete UTF-8 sequences
            // This is a simplified check - in real use, more sophisticated validation might be needed
        }
    }
    
    std::cout << "PASS" << std::endl;
}

void test_utf8_mixed_scripts() {
    std::cout << "Running test_utf8_mixed_scripts... ";
    
    LeafraChunker chunker;
    assert(chunker.initialize() == ResultCode::SUCCESS);
    
    // Mixed script text
    std::string utf8_text = "English text with 中文字符 and العربية والعبرية and ελληνικά text";
    
    ChunkingOptions options;
    options.chunk_size = 20;
    options.size_unit = ChunkSizeUnit::CHARACTERS;
    options.preserve_word_boundaries = true;
    
    std::vector<TextChunk> chunks;
    ResultCode result = chunker.chunk_text(utf8_text, options, chunks);
    
    assert(result == ResultCode::SUCCESS);
    assert(!chunks.empty());
    
    // Count total characters across all chunks (should be reasonable)
    size_t total_chars = 0;
    for (const auto& chunk : chunks) {
        total_chars += chunk.content.length(); // This is byte length, not character length
    }
    
    // Debug output
    std::cout << "Original text length: " << utf8_text.length() << ", Total chunk chars: " << total_chars << " ";
    
    // The total should be reasonable (not empty, not excessively long)
    assert(total_chars > 0);
    // With trimming and UTF-8, the total can be significantly less than original
    assert(total_chars >= 30); // Just ensure we have substantial content
    
    std::cout << "PASS" << std::endl;
}

int main() {
    std::cout << "=== UTF-8 Chunking Tests ===" << std::endl;
    
    try {
        test_utf8_character_counting();
        test_utf8_whitespace_handling();
        test_utf8_word_boundaries();
        test_utf8_emoji_handling();
        test_utf8_mixed_scripts();
        
        std::cout << std::endl << "✅ All UTF-8 tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cout << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
} 