#pragma once

#include <string>
#include <cstddef>

#ifdef LEAFRA_HAS_ICU
#include <unicode/uchar.h>
#include <unicode/utypes.h>

// macOS ICU Core has limited headers
#ifndef U_ICU_VERSION_MAJOR_NUM
// For macOS ICU Core, we need to define some constants that might not be available
#ifndef UBRK_WORD
#define UBRK_WORD 1
#endif
#endif

#else
// Fallback type for when ICU is not available
typedef int32_t UChar32;
#define U_SENTINEL -1
#endif // LEAFRA_HAS_ICU

namespace leafra {

/**
 * Safely get the Unicode code point at a given byte position in a UTF-8 string
 * @param text UTF-8 encoded string
 * @param byte_pos Byte position in the string
 * @param next_byte_pos Output: byte position of the next character
 * @return Unicode code point, or U_SENTINEL if invalid
 */
UChar32 get_unicode_char_at(const std::string& text, size_t byte_pos, size_t& next_byte_pos);

/**
 * Find the byte position of the Nth Unicode character
 * @param text UTF-8 encoded string  
 * @param char_index Index of the character to find (0-based)
 * @return Byte position of the character, or string length if out of bounds
 */
size_t get_byte_pos_for_char_index(const std::string& text, size_t char_index);

/**
 * Safely extract a substring from UTF-8 text using character positions
 * @param text UTF-8 encoded string
 * @param start_char_pos Starting character position (0-based)
 * @param char_count Number of characters to extract
 * @return Substring containing the specified characters
 */
std::string get_utf8_substring(const std::string& text, size_t start_char_pos, size_t char_count);

/**
 * Count the actual number of Unicode characters (code points) in a UTF-8 string
 * @param text UTF-8 encoded string
 * @return Number of Unicode characters
 */
size_t get_unicode_length(const std::string& text);

/**
 * Check if a Unicode character is whitespace
 * @param c Unicode code point
 * @return true if the character is whitespace
 */
bool is_unicode_whitespace(UChar32 c);

/**
 * Find word boundaries using Unicode properties
 * This is a simplified version that works with the limited ICU available on macOS
 * @param text UTF-8 encoded string
 * @param start_byte_pos Starting byte position to search from
 * @param search_forward If true, search forward; if false, search backward
 * @return Byte position of word boundary
 */
size_t find_word_boundary_helper_for_unicode(const std::string& text, size_t start_byte_pos, bool search_forward);

} // namespace leafra 