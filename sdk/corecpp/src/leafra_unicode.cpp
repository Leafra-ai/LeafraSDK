#include "leafra/leafra_unicode.h"
#include <algorithm>
#include <cctype>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <iostream>

#define DEBUG_MISMATCHES 1
// Simple debug macro to satisfy the debug logging need
#define LEAFRA_DEBUG_LOG(category, message) std::cout << "[" << category << "] " << message << std::endl

namespace leafra {

#define LEAFRA_HAS_ICU 1 // AD TEMP

#ifdef LEAFRA_HAS_ICU
// UTF-8 Helper Functions with ICU support

bool is_unicode_whitespace(UChar32 c) {
    if (c < 128) {
        // Fast ASCII branch
        return std::isspace(static_cast<int>(c));
    }
    // Fallback to full Unicode check for non-ASCII
    return u_isspace(c);
}

/**
 * Safely get the Unicode code point at a given byte position in a UTF-8 string
 * @param text UTF-8 encoded string
 * @param byte_pos Byte position in the string
 * @param next_byte_pos Output: byte position of the next character
 * @return Unicode code point, or U_SENTINEL if invalid
 * 
 */
UChar32 get_unicode_char_at(const std::string& text, size_t byte_pos, size_t& next_byte_pos) {
#ifdef LEAFRA_HAS_ICU
    // ICU implementation for proper UTF-8 handling
    const uint8_t* s = reinterpret_cast<const uint8_t*>(text.data());
    int32_t len = static_cast<int32_t>(text.length());
    int32_t i = static_cast<int32_t>(byte_pos);
    UChar32 c = 0;
    U8_NEXT(s, i, len, c);
    next_byte_pos = static_cast<size_t>(i);
    return c;
#else
    // Fallback implementation for when ICU is not available
    if (byte_pos >= text.length()) {
        next_byte_pos = text.length();
        return U_SENTINEL;
    }
    next_byte_pos = byte_pos + 1;
    return static_cast<UChar32>(static_cast<unsigned char>(text[byte_pos]));
#endif
}
//  UChar32 get_unicode_char_at_cached(const std::string& text, size_t byte_pos, size_t& next_byte_pos) {
//      return get_unicode_char_at(text, byte_pos, next_byte_pos);
//  }

// size_t hash_string(const std::string& s) {
//     return std::hash<std::string>{}(s);
// }

UChar32 get_unicode_char_at_cached(const std::string& text, size_t byte_pos, size_t& next_byte_pos) {
    struct Cache {
        std::string cached_text;
        std::vector<UChar32> codepoints_by_byte;  // maps byte_pos -> codepoint
        std::vector<size_t> next_byte_pos_by_byte; // maps byte_pos -> next_byte_pos
        int cache_initialized; 
        Cache() : cached_text(""), codepoints_by_byte(0), next_byte_pos_by_byte(0), cache_initialized(0) {}
        
    };
    static Cache cache;
    //static std::mutex cache_mutex;
    
    {
        //std::lock_guard<std::mutex> lock(cache_mutex);
        if (cache.cache_initialized == 1 && cache.cached_text != text) {
            LEAFRA_DEBUG_LOG("CACHE", "Invalid usage with different text THIS IS A BUG!");
        }
        if (cache.cache_initialized == 0 ) {
            LEAFRA_DEBUG_LOG("CACHE", "Creating new cache entry");
            //std::string preview = text.substr(0, std::min(text.size(), size_t(50)));
            //LEAFRA_DEBUG_LOG("CACHE", "Chunk preview: " + preview);
            cache.cache_initialized = 1;
            cache.cached_text = text;
            cache.codepoints_by_byte.assign(text.size() + 1, U_SENTINEL);
            cache.next_byte_pos_by_byte.assign(text.size() + 1, text.size());
            
            const uint8_t* s = reinterpret_cast<const uint8_t*>(text.data());
            int32_t len = static_cast<int32_t>(text.length());
            int32_t i = 0;
            while (i < len) {
                int32_t start = i;
                UChar32 c = 0;
                U8_NEXT(s, i, len, c);
                if (c >= 0) {
                    // Valid UTF-8 sequence
                    cache.codepoints_by_byte[start] = c;
                    cache.next_byte_pos_by_byte[start] = static_cast<size_t>(i);

                    // For continuation bytes, mark them invalid
                    for (int32_t j = start + 1; j < i; ++j) {
                        cache.codepoints_by_byte[j] = U_SENTINEL;
                        cache.next_byte_pos_by_byte[j] = j + 1;  // advance 1 byte like uncached version
                    }
                } else {
                    // Invalid sequence; mark single byte as invalid
                    cache.codepoints_by_byte[start] = U_SENTINEL;
                    cache.next_byte_pos_by_byte[start] = start + 1;
                    i = start + 1;
                }
                // If we didn't advance, force advance to prevent infinite loop
                if (i == start) {
                    i++;
                }
            } 
        }
    }
    
    size_t control_next_byte_pos = next_byte_pos; 
    UChar32 control_c = get_unicode_char_at(text, byte_pos, control_next_byte_pos);

    if (byte_pos >= text.size()) {
        next_byte_pos = text.size();

#ifdef DEBUG_MISMATCHES
        if (control_c != U_SENTINEL || control_next_byte_pos != next_byte_pos) {
            LEAFRA_DEBUG_LOG("CACHE", "MISMATCH byte_pos=" + std::to_string(byte_pos) + 
            ", text.size()=" + std::to_string(text.size()) +
            ", control_c=" + std::to_string(control_c) + 
            ", c=" + "-1" + 
            ", control_next_byte_pos=" + std::to_string(control_next_byte_pos) + 
            ", next_byte_pos=" + std::to_string(next_byte_pos));
        }
#endif
        return U_SENTINEL;
    }
    
    

    next_byte_pos = cache.next_byte_pos_by_byte[byte_pos];

#ifdef DEBUG_MISMATCHES
    if( control_c != cache.codepoints_by_byte[byte_pos] || control_next_byte_pos != next_byte_pos) {
        LEAFRA_DEBUG_LOG("CACHE", "MISMATCH byte_pos=" + std::to_string(byte_pos) + 
        ", text.size()=" + std::to_string(text.size()) +
        ", control_c=" + std::to_string(control_c) + 
        ", c=" + std::to_string(cache.codepoints_by_byte[byte_pos]) + 
        ", control_next_byte_pos=" + std::to_string(control_next_byte_pos) + 
        ", next_byte_pos=" + std::to_string(next_byte_pos));
    }
#endif
    return cache.codepoints_by_byte[byte_pos];
}




size_t get_byte_pos_for_char_index(const std::string& text, size_t char_index) {
    if (char_index == 0) return 0;
    if (text.empty()) return 0;
    
    size_t current_char = 0;
    size_t byte_pos = 0;
    while (byte_pos < text.length() && current_char < char_index) {
        size_t next_pos;
        UChar32 c = get_unicode_char_at_cached(text, byte_pos, next_pos);

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
        UChar32 c = get_unicode_char_at_cached(text, byte_pos, next_pos);
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


size_t find_word_boundary_helper_for_unicode(const std::string& text, size_t start_byte_pos, bool search_forward) {
    if (text.empty()) return 0;
    if (start_byte_pos >= text.length()) return text.length();
    
    if (search_forward) {
        // Search forward for word boundary
        size_t byte_pos = start_byte_pos;
        bool in_word = false;
        
        while (byte_pos < text.length()) {
            size_t next_pos;
            UChar32 c = get_unicode_char_at_cached(text, byte_pos, next_pos);
            
            if (c == U_SENTINEL) {
                byte_pos = next_pos;
                continue;
            }
            
            bool is_word_char = is_word_char_optimized(c);
            
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
        UChar32 c = get_unicode_char_at_cached(text, byte_pos, next_pos);
        if (c != U_SENTINEL) {
            was_in_word = is_word_char_optimized(c);
        }
        
        // Move backward character by character
        while (byte_pos > 0) {
            // Find previous character position
            size_t prev_pos = 0;
            size_t temp_pos = 0;
            
            while (temp_pos < byte_pos && temp_pos < text.length()) {
                prev_pos = temp_pos;
                UChar32 temp_c = get_unicode_char_at_cached(text, temp_pos, temp_pos);
                if (temp_c == U_SENTINEL) break;
            }
            
            UChar32 prev_c = get_unicode_char_at_cached(text, prev_pos, next_pos);
            if (prev_c == U_SENTINEL) {
                byte_pos = prev_pos;
                continue;
            }
            
            bool is_word_char = is_word_char_optimized(prev_c);
            
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