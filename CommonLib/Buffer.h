#pragma once

#include "Exception.h"
#include "CodePoint.h"
#include "Unicode.h"
#include <istream>
#include <cassert>
#include <algorithm>

namespace mws { namespace common {

template<typename CharT>
struct BufferTraits;

template<>
struct BufferTraits<char>
{
	static const char NullTerminator = '\0';
};

template<>
struct BufferTraits<wchar_t>
{
	static const wchar_t NullTerminator = L'\0';
};

template<>
struct BufferTraits<char16_t>
{
	static const char16_t NullTerminator = u'\0';
};

template<>
struct BufferTraits<char32_t>
{
	static const char32_t NullTerminator = U'\0';
};

template<typename CharT, std::size_t Size>
class BufferT
{
public:
	using Char    = CharT;
	using Traits  = BufferTraits<Char>;
	using Stream  = std::basic_istream<Char>;
	using Decoder = utf::Decoder<Char>;

	//Char=char --> utf8, Char=char16_t | wchar_t (windows) --> utf16, Char=char32_t --> utf32

	BufferT(Stream& is_)
	:
		_is(is_)
	{
		static_assert(Size >= utf::Codec<Char>::MaxCodecSizePerCP, "Buffer Size must be >= Max Codec size per CodePoint");
	}
	
	CodePoint next()
	{
		if (!valid())
			throw Exception(_C("beyond end of input"));

		//++_pos;
	
		// reached end of buffer?
		//if (pos() == _posReload)
		const auto diff = _posReload - pos();
		// a unicode CodePoint may be encoded with MaxSize code units
		if (diff < utf::Codec<Char>::MaxCodecSizePerCP)
		{
			assert(idx() <= Size);

			//load next buffer
			const auto offset = idx() + diff;
			_is.read(&_buf[offset], Size);
			_buf[offset + _is.gcount()] = Traits::NullTerminator;

			//move reload point forward
			_posReload += Size;
		}

		const auto size = Decoder::decode(&_buf[idx()], _curCP);
		_pos += size;

		return _curCP;//cur();
	}

	std::size_t pos() const
	{
		return _pos;
	}

	void retract(std::size_t diff_)
	{
		if (diff_ > std::min(pos(), Size))
		{
			throw Exception(_C("cannot retract this far back"));
		}

		_pos -= diff_;
	}

	bool valid() const
	{
		//TODO: cur() breaks when retracting
		return _is || cur() != Traits::NullTerminator;
	}

private:

	CodePoint cur() const
	{
		//assert(_pos != -1);
		//return _buf[idx()];
		assert(_curCP != -1);
		return _curCP;
	}

	std::size_t idx() const
	{
		return _pos % (Size * 2);
	}

private:
	Char        _buf[2 * Size + 8];
	Stream&     _is;
	std::size_t _pos = 0;
	std::size_t _posReload = 0;
	CodePoint   _curCP = -1;
};

using BufferExt = BufferT<CharExt, 4096>;

}}
