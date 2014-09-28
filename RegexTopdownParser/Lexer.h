#pragma once

#include "Exception.h"
#include <boost/circular_buffer.hpp>
#include <string>
#include <sstream>

template<typename Buffer>
class Lexer
{
public:
	Lexer(std::istream& is_)
		:
		_buf(is_), _tokenBuf(256), _count(0), _cur(0)
	{
		_tokenBuf.push_back(_buf.cur());
	}

	// super simple lexer. Each character is a token
	void next()
	{
		if (_tokenBuf[_cur] == '\0')
			throw Exception("Lexer eof");

		++_cur; ++_count;

		if (_cur == _tokenBuf.size())
		{
			_buf.next();
			_tokenBuf.push_back(_buf.cur());
		}

		_cur = std::min(_tokenBuf.capacity() - 1, _cur);
	}

	char cur() const
	{
		return _tokenBuf[_cur];
	}

	std::size_t count() const
	{
		return _count;
	}

	void retract(std::size_t diff_)
	{
		if (diff_ > _cur)
			throw Exception("cannot retract pos this far back");

		_cur -= diff_; _count -= diff_;
	}

private:
	Buffer _buf;
	boost::circular_buffer<char> _tokenBuf;
	std::size_t _count;
	std::size_t _cur;
};
