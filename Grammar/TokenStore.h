#pragma once

#include <CommonLib/TokenID.h>
#include <cassert>

namespace mws { namespace grammar {

struct Token;

class TokenStore
{
public:
	const Token* get(TokenID index_) const
	{
		return _tokens[index_];
	}

	const Token* pop(TokenID index_)
	{
		const auto* t = get(index_);
		_tokens[index_] = nullptr;
		return t;
	}

	void  put(TokenID index_, const Token* t_)
	{
		_tokens[index_] = t_;
	}

private:
	const Token* _tokens[256];
};

}}
