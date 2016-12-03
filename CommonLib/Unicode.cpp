#include "Unicode.h"

namespace mws { namespace common {

const uint8_t UTF8_LEAD_BYTE1 = 0b00000000;
const uint8_t UTF8_LEAD_BYTE2 = 0b11000000;
const uint8_t UTF8_LEAD_BYTE3 = 0b11100000;
const uint8_t UTF8_LEAD_BYTE4 = 0b11110000;
const uint8_t UTF8_TRAIL_BYTE = 0b10000000;

extern const uint16_t UTF16_HS_LEAD = 0xD800;
extern const uint16_t UTF16_LS_LEAD = 0xDC00;

// Note: brace elision (the braces around Utf8LeadByte pairs are elided) required to compile this.
const Utf8LeadByteVec g_utf8LeadByteVec =
{
    UTF8_LEAD_BYTE1, 0b10000000,
    UTF8_LEAD_BYTE2, 0b11100000,
    UTF8_LEAD_BYTE3, 0b11110000,
    UTF8_LEAD_BYTE4, 0b11111000
};

}}
