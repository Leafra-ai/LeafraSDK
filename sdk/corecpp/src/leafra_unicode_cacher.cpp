#include "leafra/leafra_unicode.h"
#include <algorithm>
#include <cctype>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <iostream>

// Simple debug macro to satisfy the debug logging need
#define LEAFRA_DEBUG_LOG(category, message) std::cout << "[" << category << "] " << message << std::endl

#undef DEBUG_MISMATCHES 

namespace leafra {

#define LEAFRA_HAS_ICU 1 // AD TEMP

#ifdef LEAFRA_HAS_ICU
#include <unicode/uchar.h>
#include <unicode/utypes.h>
#include <unicode/utf8.h>
#endif

// UnicodeCacher member function implementations

void UnicodeCacher::initialize_cache(const std::string& text) {
    //LEAFRA_DEBUG_LOG("CACHE", "Creating new cache entry");
    
    cached_text = text;

    if (cached_text.empty()) cached_unicode_length = 0;


    codepoints_by_byte.assign(text.size() + 1, U_SENTINEL);
    next_byte_pos_by_byte.assign(text.size() + 1, text.size());
    
    const uint8_t* s = reinterpret_cast<const uint8_t*>(text.data());
    int32_t len = static_cast<int32_t>(text.length());
    int32_t i = 0;

    while (i < len) {
        int32_t start = i;
        UChar32 c = 0;
        U8_NEXT(s, i, len, c);
        if (c >= 0) {
            // Valid UTF-8 sequence
            codepoints_by_byte[start] = c;
            next_byte_pos_by_byte[start] = static_cast<size_t>(i);

            // For continuation bytes, mark them invalid
            for (int32_t j = start + 1; j < i; ++j) {
                codepoints_by_byte[j] = U_SENTINEL;
                next_byte_pos_by_byte[j] = j + 1;  // advance 1 byte like uncached version
            }
        } else {
            // Invalid sequence; mark single byte as invalid
            codepoints_by_byte[start] = U_SENTINEL;
            next_byte_pos_by_byte[start] = start + 1;
            i = start + 1;
        }
        // If we didn't advance, force advance to prevent infinite loop
        if (i == start) {
            i++;
        }
    }
        
    size_t char_count = 0;
    size_t byte_pos = 0;
    
    while (byte_pos < cached_text.length()) {
        size_t next_pos;
        UChar32 c = get_unicode_char_at_cached(byte_pos, next_pos);
        if (c != U_SENTINEL) {
            char_count++;
        }
        byte_pos = next_pos;
        
        // Safety check to prevent infinite loops
        if (next_pos <= byte_pos) {
            byte_pos++;
        }
    }
    cached_unicode_length = char_count;
}

UnicodeCacher::UnicodeCacher() : cached_text(""), codepoints_by_byte(0), next_byte_pos_by_byte(0), cached_unicode_length(0) {}

UnicodeCacher::UnicodeCacher(const std::string& text) : cached_text(""), codepoints_by_byte(0), next_byte_pos_by_byte(0), cached_unicode_length(0)   {
    initialize_cache(text);
}

void UnicodeCacher::reinitialize(const std::string& text) {
    initialize_cache(text);
}

UChar32 UnicodeCacher::get_unicode_char_at_cached(size_t byte_pos, size_t& next_byte_pos) const {
#ifdef DEBUG_MISMATCHES
    size_t control_next_byte_pos = next_byte_pos; 
    UChar32 control_c = get_unicode_char_at(cached_text, byte_pos, control_next_byte_pos);
#endif 
    if (byte_pos >= cached_text.size()) {
        next_byte_pos = cached_text.size();

#ifdef DEBUG_MISMATCHES
        if (control_c != U_SENTINEL || control_next_byte_pos != next_byte_pos) {
            LEAFRA_DEBUG_LOG("CACHE", "MISMATCH byte_pos=" + std::to_string(byte_pos) + 
            ", text.size()=" + std::to_string(cached_text.size()) +
            ", control_c=" + std::to_string(control_c) + 
            ", c=" + "-1" + 
            ", control_next_byte_pos=" + std::to_string(control_next_byte_pos) + 
            ", next_byte_pos=" + std::to_string(next_byte_pos));
        }
#endif
        return U_SENTINEL;
    }

    next_byte_pos = next_byte_pos_by_byte[byte_pos];

#ifdef DEBUG_MISMATCHES
    if( control_c != codepoints_by_byte[byte_pos] || control_next_byte_pos != next_byte_pos) {
        LEAFRA_DEBUG_LOG("CACHE", "MISMATCH byte_pos=" + std::to_string(byte_pos) + 
        ", text.size()=" + std::to_string(cached_text.size()) +
        ", control_c=" + std::to_string(control_c) + 
        ", c=" + std::to_string(codepoints_by_byte[byte_pos]) + 
        ", control_next_byte_pos=" + std::to_string(control_next_byte_pos) + 
        ", next_byte_pos=" + std::to_string(next_byte_pos));
    }
#endif
    return codepoints_by_byte[byte_pos];
}

size_t UnicodeCacher::get_byte_pos_for_char_index_cached(size_t char_index) const {
    if (char_index == 0) return 0;
    if (cached_text.empty()) return 0;
    
    size_t current_char = 0;
    size_t byte_pos = 0;
    while (byte_pos < cached_text.length() && current_char < char_index) {
        size_t next_pos;
        UChar32 c = get_unicode_char_at_cached(byte_pos, next_pos);

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

std::string UnicodeCacher::get_utf8_substring_cached(size_t start_char_pos, size_t char_count) const {
    if (cached_text.empty() || char_count == 0) return "";
    
    size_t start_byte_pos = get_byte_pos_for_char_index_cached(start_char_pos);
    if (start_byte_pos >= cached_text.length()) return "";
    
    size_t end_char_pos = start_char_pos + char_count;
    size_t end_byte_pos = get_byte_pos_for_char_index_cached(end_char_pos);
    
    if (start_byte_pos >= end_byte_pos) return "";
    
    return cached_text.substr(start_byte_pos, end_byte_pos - start_byte_pos);
}

size_t UnicodeCacher::get_unicode_length_cached() const {
    return cached_unicode_length;
}


size_t UnicodeCacher::find_word_boundary_helper_for_unicode_cached(size_t start_byte_pos, bool search_forward) const {
    if (cached_text.empty()) return 0;
    if (start_byte_pos >= cached_text.length()) return cached_text.length();
    
    if (search_forward) {
        // Search forward for word boundary
        size_t byte_pos = start_byte_pos;
        bool in_word = false;
        
        while (byte_pos < cached_text.length()) {
            size_t next_pos;
            UChar32 c = get_unicode_char_at_cached(byte_pos, next_pos);
            
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
        
        return cached_text.length();
    } else {
        // Search backward for word boundary
        if (start_byte_pos == 0) return 0;
        
        size_t byte_pos = start_byte_pos;
        bool was_in_word = false;
        
        // Check current position first
        size_t next_pos;
        UChar32 c = get_unicode_char_at_cached(byte_pos, next_pos);
        if (c != U_SENTINEL) {
            was_in_word = is_word_char_optimized(c);
        }
        
        // Move backward character by character
        while (byte_pos > 0) {
            // Find previous character position
            size_t prev_pos = 0;
            size_t temp_pos = 0;
            
            while (temp_pos < byte_pos && temp_pos < cached_text.length()) {
                prev_pos = temp_pos;
                UChar32 temp_c = get_unicode_char_at_cached(temp_pos, temp_pos);
                if (temp_c == U_SENTINEL) break;
            }
            
            UChar32 prev_c = get_unicode_char_at_cached(prev_pos, next_pos);
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


} // namespace leafra 