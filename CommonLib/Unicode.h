#pragma once

#include "UnicodeException.h"
#include "CodePoint.h"
//#include <Winsock2.h>
#include <array>
#include <cassert>

namespace mws { namespace common {

extern const uint8_t UTF8_LEAD_BYTE1;
extern const uint8_t UTF8_LEAD_BYTE2;
extern const uint8_t UTF8_LEAD_BYTE3;
extern const uint8_t UTF8_LEAD_BYTE4;
extern const uint8_t UTF8_TRAIL_BYTE;

extern const uint16_t UTF16_HS_LEAD;
extern const uint16_t UTF16_LS_LEAD;

struct Utf8LeadByte
{
    uint8_t _byte;
    uint8_t _mask;
};

using Utf8LeadByteVec = std::array<Utf8LeadByte, 4>;
extern const Utf8LeadByteVec g_utf8LeadByteVec;

struct Unicode
{
    struct CodeSpace
    {
        static const CodePoint min = 0x00000000;
        static const CodePoint max = 0x0010FFFF;
    };

    // CodeSpace is divided in 64k planes. Plane 0 is BMP (Basic Multilingual Plane)
    // Lower 4 bytes of a codepoint give position in place. The higher bytes give plane.

    // Basic Multilingual plane (Plane 0)
    // 0x00, 0x7F is ASCII, 0x80, 0xFF is Latin 1
    struct Plane0
    {
        static const CodePoint min = 0x00000000;
        static const CodePoint max = 0x0000FFFF;
    };

    using BMP = Plane0;

    // CodePoints for use in UTF16 encoding. Not valid CodePoint scalar values
    struct HighSurrogate
    {
        static const CodePoint min = 0x0000D800;
        static const CodePoint max = 0x0000DBFF;
    };

    using HSurrogate = HighSurrogate;

    struct LowSurrogate
    {
        static const CodePoint min = 0x0000DC00;
        static const CodePoint max = 0x0000DFFF;
    };

    using LSurrogate = LowSurrogate;

    struct Surrogate
    {
        static const CodePoint min = HSurrogate::min;
        static const CodePoint max = LSurrogate::max;
    };

    // Supplementary Multilingual plane (Plan 1)
    struct Plane1
    {
        static const CodePoint min = 0x00010000;
        static const CodePoint max = 0x0001FFFF;
    };

    using SMP = Plane1;

    // Supplementary Ideographic plane (Plan 2)
    struct Plane2
    {
        static const CodePoint min = 0x00020000;
        static const CodePoint max = 0x0002FFFF;
    };

    using SIP = Plane2;

    // codepoints with Default_Ignorable_CodePoint property
    struct Plane14
    {
        static const CodePoint min = 0x000E0000;
        static const CodePoint max = 0x000E1000;
    };
};

inline bool isSurrogate(CodePoint cp_)
{
    return cp_ >= Unicode::Surrogate::min && cp_ <= Unicode::Surrogate::max;
}

inline int getUTF8ByteCount(CodePoint cp_)
{
    if (cp_ <= 0x0000007F)
        return 1;

    if (cp_ <= 0x000007FF)
        return 2;

    if (cp_ <= 0x0000FFFF)
        return 3;

    if (cp_ <= 0x001FFFFF)
        return 4;

    return 0;
}

inline int getUTF16ByteCount(CodePoint cp_)
{
    if (cp_ <= 0x00000D7FF)
        return 1;

	//Note: The code points in the range D800, D8FF are used
	//for surrogate pairs. These are not valid code point 'scalar values'

    if (cp_ >= 0x0000E000 && cp_ <= 0x0000FFFF)
        return 1;

    if (cp_ >= 0x00010000 && cp_ <= 0x0010FFFF)
        return 2;

    return 0;
}

inline int codePointToUTF8(CodePoint cp_, char* buf_)
{
    if (isSurrogate(cp_) || cp_ > 0x001FFFFF)
    {
        throw EncodingException();
    }

    const auto count = getUTF8ByteCount(cp_);
    assert(count > 0);

    for (auto i = count - 1; i > 0; --i)
    {
        buf_[i] = static_cast<char>((cp_ & 0b00111111) | UTF8_TRAIL_BYTE);
        cp_ >>= 6;
    }

    buf_[0] = static_cast<char>(cp_ | g_utf8LeadByteVec[count - 1]._byte);
    return count;
}

inline int codePointFromUTF8(const char* buf_, CodePoint& cp_)
{
    for (auto i = 0; i < g_utf8LeadByteVec.size(); ++i)
    {
		//important! char is signed. without the intemediate unsigned cast
		//negative chars (> 0x7F) convert to negative int
		const auto c = static_cast<unsigned char>(buf_[0]);

        if ((c & g_utf8LeadByteVec[i]._mask) != g_utf8LeadByteVec[i]._byte)
            continue;

        // leading byte
        cp_ = c & ~g_utf8LeadByteVec[i]._mask;
        // trailing bytes
        for (auto j = 1; j <= i; ++j)
        {
			const auto c = static_cast<unsigned char>(buf_[j]);

            if ((c & 0b11000000) != UTF8_TRAIL_BYTE)
                throw DecodingException();

            cp_ <<= 6;
            cp_ |= c & 0b00111111;
        }

        return i + 1;
    }

    throw DecodingException();
}

inline int codePointToUTF16(CodePoint cp_, char16_t* buf_)
{
    if (isSurrogate(cp_) || cp_ > 0x0010FFFF)
    {
        throw EncodingException();
    }

    const auto count = getUTF16ByteCount(cp_);
    assert(count > 0);

    if (count == 1)
    {
        buf_[0] = static_cast<char16_t>(cp_);
    }
    else
    {
        // subtraction leaves us with 20 bits
        cp_ -= (1 << 16);
        assert(cp_ <= 0x000FFFFF);

        // low surrogate code unit
        buf_[1] = (cp_ & 0x03FF)              | UTF16_LS_LEAD;
        // high surrogate code unit
        cp_ >>= 10;
        assert((cp_ & ~0x03FF) == 0x0);
        buf_[0] =  static_cast<char16_t>(cp_) | UTF16_HS_LEAD;
    }

    return count;
}

inline int codePointToUTF16_LE(CodePoint cp_, char* buf_)
{
    // performance alignment check
    assert(reinterpret_cast<uint64_t>(buf_) % sizeof(char16_t) == 0);

    auto count = codePointToUTF16(cp_, reinterpret_cast<char16_t*>(buf_));
    //network byte order is big endian: most significant byte stored first
    for (auto i = 0; i < count; ++i)
    {
        uint16_t& c = *reinterpret_cast<uint16_t*>(buf_);
        // from host to big endian
		assert(false && "TODO: replace htons with own version to prevent Winsock2 include (or put this function in cpp file to localize Winsock2 include");
        //c = htons(c);
        // from big endian to little endian
        std::swap(buf_[0], buf_[1]);
        buf_ += sizeof(uint16_t);
    }
}

inline int codePointFromUTF16(const char16_t* buf_, CodePoint& cp_)
{
    if (buf_[0] < Unicode::Surrogate::min)
    {
        cp_ = buf_[0];
        return 1;
    }

    if (buf_[0] > Unicode::Surrogate::max && buf_[0] < 0xFFFF)
    {
        cp_ = buf_[0];
        return 1;
    }

    // low and high surrogate code unit must be well formed
    if ((buf_[1] & ~0x03FF) == UTF16_LS_LEAD && (buf_[0] & ~0x03FF) == UTF16_HS_LEAD)
    {
        cp_  = buf_[0] & 0x03FF;
        cp_ <<= 10;
		cp_ += (1 << 16);
        cp_ |= buf_[1] & 0x03FF;
        return 2;
    }

    throw DecodingException();
}

inline int codePointToUTF32(CodePoint cp_, char32_t* buf_)
{
    buf_[0] = cp_;
    return 1;
}

inline int codePointFromUTF32(const char32_t* buf_, CodePoint& cp_)
{
    cp_ = buf_[0];
    return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////

namespace utf {

template<typename CharT>
struct Encoder;

template<>
struct Encoder<char>
{
	static std::size_t encode(CodePoint cp_, char* buf_)
	{
		return codePointToUTF8(cp_, buf_);
	}
};

template<>
struct Encoder<wchar_t>
{
	static std::size_t encode(CodePoint cp_, wchar_t* buf_)
	{
		return codePointToUTF16(cp_, reinterpret_cast<char16_t*>(buf_));
	}
};

template<>
struct Encoder<char16_t>
{
	static std::size_t encode(CodePoint cp_, char16_t* buf_)
	{
		return codePointToUTF16(cp_, buf_);
	}
};

template<>
struct Encoder<char32_t>
{
	static std::size_t encode(CodePoint cp_, char32_t* buf_)
	{
		return codePointToUTF32(cp_, buf_);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

template<typename CharT>
struct Decoder;

template<>
struct Decoder<char>
{
	static std::size_t decode(const char* buf_, CodePoint& cp_)
	{
		return codePointFromUTF8(buf_, cp_);
	}
};

template<>
struct Decoder<wchar_t>
{
	static std::size_t decode(const wchar_t* buf_, CodePoint& cp_)
	{
		return codePointFromUTF16(reinterpret_cast<const char16_t*>(buf_), cp_);
	}
};

template<>
struct Decoder<char16_t>
{
	static std::size_t decode(const char16_t* buf_, CodePoint& cp_)
	{
		return codePointFromUTF16(buf_, cp_);
	}
};

template<>
struct Decoder<char32_t>
{
	static std::size_t decode(const char32_t* buf_, CodePoint& cp_)
	{
		return codePointFromUTF32(buf_, cp_);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

template<typename CharT>
struct Codec
:
	Encoder<CharT>,
	Decoder<CharT>
{
	static constexpr auto MaxCodecSizePerCP = 4 / sizeof(CharT);
};

} //utf

}} //mws::common

