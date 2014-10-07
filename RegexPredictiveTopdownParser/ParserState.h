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

	std::size_t pos() const
	{
		return _lexer.pos();
	}

	BufferT& buf()
	{
		return _buf;
	}

	void retract(std::size_t pos_, const Token& t_)
	{
		_lexer.retract(pos() - pos_);
		_cur = t_;
	}

	//Note: match does not assert/throw on this->pos() == eof() because this parser allows for backtracking. If the current production fails, there may
	//be an empty() production that makes the parse succeed without further matching.
	//If no empty() production exists, a false result propagates back up to the toplevel invocation.

	bool m(Token::Type type_)
	{
		bool match = false;
		if (!eof())
		{
			//Note: only get next() on successfull match to ensure we don't 'lex' past a token marking a different lexicon (requiering a lexer switch)
			//In particular: the '[' token can only successfully match in combination with a lexer switch.
			if (match = _cur._type == type_)
			{
				_cur = _lexer.next();
			}
		}

		return match;
	}

private:
	BufferT& _buf;
	Token&   _cur;
	LexerT   _lexer;
};

}}}
