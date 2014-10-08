#pragma once

#include "Token.h"
#include <cassert>

namespace mws { namespace td { namespace LL1 {

template<typename BufferT, typename LexerT>
class ParserState
{
public:
	ParserState(BufferT& buf_, Token& cur_)
		: _buf(buf_), _cur(cur_), _lexer(buf_)
	{
		
	}

	void init()
	{
		//load first token
		_cur = _lexer.next();
	}

	bool eof() const
	{
		return _cur._type == Token::Type::Eof;
	}

	Token& cur() const
	{
		return _cur;
	}

	BufferT& buf()
	{
		return _buf;
	}

	void m(Token::Type type_)
	{
		if (eof() || _cur._type != type_)
		{
			throw common::Exception("Lexer error");
		}

		_cur = _lexer.next();
	}

private:
	BufferT& _buf;
	Token&   _cur;
	LexerT   _lexer;
};

}}}
