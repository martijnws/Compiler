#pragma once

#include "Token.h"
#include <cassert>

namespace mws { namespace td { namespace pr {

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

	bool empty() const
	{
		return true;
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

	//Note: match does not assert/throw on this->pos() == eof() because this parser allows for backtracking. If the current production fails, there may
	//be an empty() production that makes the parse succeed without further matching.
	//If no empty() production exists, a false result propagates back up to the toplevel invocation.

	//Note: a match can only fail if there is a grammar error in the input.
	bool m(Token::Type type_)
	{
		if (!eof() && _cur._type == type_)
		{
			//Note: only get next() on successfull match to ensure we don't 'lex' past a token marking a different lexicon (requiering a lexer switch)
			//In particular: the '[' token can only successfully match in combination with a lexer switch.
			_cur = _lexer.next();
			return true;
		}

		return false;
	}

private:
	BufferT& _buf;
	Token&   _cur;
	LexerT   _lexer;
};

}}}
