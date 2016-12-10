#pragma once

#include "Token.h"
#include <cassert>

namespace mws { namespace grammar {

class TokenStore
{
public:
	const Token* get(uint8_t index_) const
	{
		return _tokens[index_];
	}

	const Token* pop(uint8_t index_)
	{
		const auto* t = get(index_);
		_tokens[index_] = nullptr;
		return t;
	}

	void  put(uint8_t index_, const Token* t_)
	{
		_tokens[index_] = t_;
	}

private:
	const Token* _tokens[256];
};

}}
