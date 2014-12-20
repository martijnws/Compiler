#pragma once

#include "Token.h"
#include <cassert>

namespace mws { namespace grammar {

class TokenStore
{
public:
	Token get(uint8_t index_) const
	{
		return _tokens[index_];
	}

	Token pop(uint8_t index_)
	{
		Token t = get(index_);
		_tokens[index_]._lexeme = 0;
		_tokens[index_]._type = Token::None;
		return t;
	}

	void  put(uint8_t index_, const Token& t_)
	{
		assert(_tokens[index_]._type != Token::None);
		_tokens[index_] = t_;
	}

private:
	Token _tokens[256];
};

}}
