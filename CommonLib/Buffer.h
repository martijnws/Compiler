#pragma once

#include "Exception.h"
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
	static const char NullTerminator = L'\0';
};

template<typename CharT, std::size_t Size>
class BufferT
{
public:
	using Traits = BufferTraits<CharT>;

	BufferT(std::istream& is_)
		:
		_is(is_), _pos(-1), _posReload(0)
	{
		
	}
	
	char cur() const
	{
		assert(_pos != -1);
		return _buf[idx()];
	}

	char next()
	{
		if (!valid())
			throw Exception("beyond end of input");

		++_pos;
	
		// reached end of buffer?
		if (pos() == _posReload)
		{
			assert(idx() <= Size);

			//load next buffer
			_is.read(&_buf[idx()], Size);
			_buf[idx() + _is.gcount()] = Traits::NullTerminator;

			//move reload point forward
			_posReload += Size;
		}

		return cur();
	}

	std::size_t pos() const
	{
		return _pos;
	}

	void retract(std::size_t diff_)
	{
		if (diff_ > std::min(pos() + 1, Size))
			throw Exception("cannot retract this far back");

		_pos -= diff_;
	}

	bool valid() const
	{
		return _is || cur() != Traits::NullTerminator;
	}

private:

	std::size_t idx() const
	{
		return _pos % (Size * 2);
	}

private:
	char _buf[2 * Size + 8];
	std::istream& _is;
	std::size_t _pos;
	std::size_t _posReload;
};

using Buffer = BufferT<char, 4096>;

}}
