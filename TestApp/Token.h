#pragma once

#include <Grammar/Token.h>
#include <CommonLib/TokenID.h>
#include <CommonLib/String.h>

namespace mws { namespace arith {

struct Token
:
	public grammar::Token
{
	enum Enum { Add, Num, Eof, Invalid };

	static constexpr TokenID max()
	{
		return Invalid;
	}

	bool isLast() const
	{
		return _type == Enum::Eof;
	}

	String  _lexeme;
	Enum    _type;
};

}}
