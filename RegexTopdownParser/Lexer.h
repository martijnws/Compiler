#pragma once

template<typename Buffer>
class Lexer
{
public:
	Lexer(std::istream& is_)
		:
		_buf(is_)
	{

	}

	// super simple lexer. Each character is a token
	char next()
	{
		char n = _buf.fwd();
		_buf.syncCur();
		return n;
	}

private:
	Buffer _buf;
};
