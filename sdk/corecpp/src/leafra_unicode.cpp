#include "leafra/leafra_unicode.h"
#include <algorithm>
#include <cctype>

namespace leafra {

#ifdef LEAFRA_HAS_ICU
// UTF-8 Helper Functions with ICU support

UChar32 get_unicode_char_at(const std::string& text, size_t byte_pos, size_t& next_byte_pos) {
    if (byte_pos >= text.length()) {
        next_byte_pos = text.length();
        return U_SENTINEL;
    }
    
    // Manual UTF-8 decoding - more reliable than ICU macros on macOS ICU Core
    const char* str = text.c_str();
    const char* ptr = str + byte_pos;
    const char* end = str + text.length();
    
    if (ptr >= end) {
        next_byte_pos = text.length();
        return U_SENTINEL;
    }
    
    unsigned char c1 = static_cast<unsigned char>(*ptr);
    
    if (c1 < 0x80) {
        // ASCII character
        next_byte_pos = byte_pos + 1;
        return static_cast<UChar32>(c1);
    } else if ((c1 & 0xE0) == 0xC0 && ptr + 1 < end) {
        // 2-byte sequence
        unsigned char c2 = static_cast<unsigned char>(ptr[1]);
        if ((c2 & 0xC0) == 0x80) {
            next_byte_pos = byte_pos + 2;
            return ((c1 & 0x1F) << 6) | (c2 & 0x3F);
        }
    } else if ((c1 & 0xF0) == 0xE0 && ptr + 2 < end) {
        // 3-byte sequence
        unsigned char c2 = static_cast<unsigned char>(ptr[1]);
        unsigned char c3 = static_cast<unsigned char>(ptr[2]);
        if ((c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80) {
            next_byte_pos = byte_pos + 3;
            return ((c1 & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
        }
    } else if ((c1 & 0xF8) == 0xF0 && ptr + 3 < end) {
        // 4-byte sequence
        unsigned char c2 = static_cast<unsigned char>(ptr[1]);
        unsigned char c3 = static_cast<unsigned char>(ptr[2]);
        unsigned char c4 = static_cast<unsigned char>(ptr[3]);
        if ((c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80 && (c4 & 0xC0) == 0x80) {
            next_byte_pos = byte_pos + 4;
            return ((c1 & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
        }
    }
    
    // Invalid UTF-8 sequence, advance by one byte
    next_byte_pos = byte_pos + 1;
    return U_SENTINEL;
}

size_t get_byte_pos_for_char_index(const std::string& text, size_t char_index) {
    if (char_index == 0) return 0;
    if (text.empty()) return 0;
    
    size_t current_char = 0;
    size_t byte_pos = 0;
    
    while (byte_pos < text.length() && current_char < char_index) {
        size_t next_pos;
        UChar32 c = get_unicode_char_at(text, byte_pos, next_pos);
        if (c != U_SENTINEL) {
            current_char++;
        }
        byte_pos = next_pos;
        
        // Safety check
        if (next_pos <= byte_pos) {
            byte_pos++;
        }
    }
    
    return byte_pos;
}

std::string get_utf8_substring(const std::string& text, size_t start_char_pos, size_t char_count) {
    if (text.empty() || char_count == 0) return "";
    
    size_t start_byte_pos = get_byte_pos_for_char_index(text, start_char_pos);
    if (start_byte_pos >= text.length()) return "";
    
    size_t end_char_pos = start_char_pos + char_count;
    size_t end_byte_pos = get_byte_pos_for_char_index(text, end_char_pos);
    
    if (start_byte_pos >= end_byte_pos) return "";
    
    return text.substr(start_byte_pos, end_byte_pos - start_byte_pos);
}

size_t get_unicode_length(const std::string& text) {
    if (text.empty()) return 0;
    
    size_t char_count = 0;
    size_t byte_pos = 0;
    
    while (byte_pos < text.length()) {
        size_t next_pos;
        UChar32 c = get_unicode_char_at(text, byte_pos, next_pos);
        if (c != U_SENTINEL) {
            char_count++;
        }
        byte_pos = next_pos;
        
        // Safety check to prevent infinite loops
        if (next_pos <= byte_pos) {
            byte_pos++;
        }
    }
    
    return char_count;
}

bool is_unicode_whitespace(UChar32 c) {
    return u_isspace(c);
}

size_t find_word_boundary_helper_for_unicode(const std::string& text, size_t start_byte_pos, bool search_forward) {
    if (text.empty()) return 0;
    if (start_byte_pos >= text.length()) return text.length();
    
    if (search_forward) {
        // Search forward for word boundary
        size_t byte_pos = start_byte_pos;
        bool in_word = false;
        
        while (byte_pos < text.length()) {
            size_t next_pos;
            UChar32 c = get_unicode_char_at(text, byte_pos, next_pos);
            
            if (c == U_SENTINEL) {
                byte_pos = next_pos;
                continue;
            }
            
            bool is_word_char = u_isalnum(c) || c == '_';
            
            if (in_word && !is_word_char) {
                // End of word found
                return byte_pos;
            }
            
            in_word = is_word_char;
            byte_pos = next_pos;
        }
        
        return text.length();
    } else {
        // Search backward for word boundary
        if (start_byte_pos == 0) return 0;
        
        size_t byte_pos = start_byte_pos;
        bool was_in_word = false;
        
        // Check current position first
        size_t next_pos;
        UChar32 c = get_unicode_char_at(text, byte_pos, next_pos);
        if (c != U_SENTINEL) {
            was_in_word = u_isalnum(c) || c == '_';
        }
        
        // Move backward character by character
        while (byte_pos > 0) {
            // Find previous character position
            size_t prev_pos = 0;
            size_t temp_pos = 0;
            
            while (temp_pos < byte_pos && temp_pos < text.length()) {
                prev_pos = temp_pos;
                UChar32 temp_c = get_unicode_char_at(text, temp_pos, temp_pos);
                if (temp_c == U_SENTINEL) break;
            }
            
            UChar32 prev_c = get_unicode_char_at(text, prev_pos, next_pos);
            if (prev_c == U_SENTINEL) {
                byte_pos = prev_pos;
                continue;
            }
            
            bool is_word_char = u_isalnum(prev_c) || prev_c == '_';
            
            if (was_in_word && !is_word_char) {
                // Found word boundary
                return byte_pos;
            }
            
            was_in_word = is_word_char;
            byte_pos = prev_pos;
        }
        
        return 0;
    }
}

#else
// Fallback functions for when ICU is not available

UChar32 get_unicode_char_at(const std::string& text, size_t byte_pos, size_t& next_byte_pos) {
    if (byte_pos >= text.length()) {
        next_byte_pos = text.length();
        return U_SENTINEL;
    }
    next_byte_pos = byte_pos + 1;
    return static_cast<UChar32>(static_cast<unsigned char>(text[byte_pos]));
}

size_t get_byte_pos_for_char_index(const std::string& text, size_t char_index) {
    if (char_index == 0) return 0;
    if (text.empty()) return 0;
    
    size_t current_char = 0;
    size_t byte_pos = 0;
    
    while (byte_pos < text.length() && current_char < char_index) {
        size_t next_pos;
        UChar32 c = get_unicode_char_at(text, byte_pos, next_pos);
        if (c != U_SENTINEL) {
            current_char++;
        }
        byte_pos = next_pos;
        
        // Safety check
        if (next_pos <= byte_pos) {
            byte_pos++;
        }
    }
    
    return byte_pos;
}

std::string get_utf8_substring(const std::string& text, size_t start_char_pos, size_t char_count) {
    // Fallback: assume byte positions equal character positions
    size_t start_pos = std::min(start_char_pos, text.length());
    size_t end_pos = std::min(start_pos + char_count, text.length());
    return text.substr(start_pos, end_pos - start_pos);
}

size_t get_unicode_length(const std::string& text) {
    return text.length(); // Fallback to byte length
}

bool is_unicode_whitespace(UChar32 c) {
    return std::isspace(static_cast<int>(c));
}

size_t find_word_boundary_helper_for_unicode(const std::string& text, size_t start_byte_pos, bool search_forward) {
    // Fallback to simple ASCII-based word boundary detection
    if (search_forward) {
        for (size_t i = start_byte_pos; i < text.length(); ++i) {
            if (std::isspace(text[i])) {
                return i;
            }
        }
        return text.length();
    } else {
        for (size_t i = start_byte_pos; i > 0; --i) {
            if (std::isspace(text[i - 1])) {
                return i;
            }
        }
        return 0;
    }
}

#endif // LEAFRA_HAS_ICU

} // namespace leafra 