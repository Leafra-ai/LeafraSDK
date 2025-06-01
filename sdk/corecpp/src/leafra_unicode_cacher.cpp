#include "leafra/leafra_unicode.h"
#include <algorithm>
#include <cctype>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <iostream>

// Simple debug macro to satisfy the debug logging need
#define LEAFRA_DEBUG_LOG(category, message) std::cout << "[" << category << "] " << message << std::endl

#define DEBUG_MISMATCHES 1

namespace leafra {

#define LEAFRA_HAS_ICU 1 // AD TEMP

#ifdef LEAFRA_HAS_ICU
#include <unicode/uchar.h>
#include <unicode/utypes.h>
#include <unicode/utf8.h>
#endif

// UnicodeCacher member function implementations

void UnicodeCacher::initialize_cache(const std::string& text) {
    LEAFRA_DEBUG_LOG("CACHE", "Creating new cache entry");
    
    cached_text = text;
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
}

UnicodeCacher::UnicodeCacher() : cached_text(""), codepoints_by_byte(0), next_byte_pos_by_byte(0) {}

UnicodeCacher::UnicodeCacher(const std::string& text) : cached_text(""), codepoints_by_byte(0), next_byte_pos_by_byte(0) {
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

} // namespace leafra 