#pragma once

#include <Grammar/Token.h>
#include <CommonLib/TokenID.h>
#include <CommonLib/CodePoint.h>

namespace mws { namespace regex {

struct REToken
:
	public grammar::Token
{
	enum Enum : TokenID { Invalid, Eof, Symbol, Choice, ZeroOrOne, ZeroToMany, OneToMany, SubExprB, SubExprE, CharClassB, CharClassE };

	static TokenID max()
	{
		return Enum::CharClassE;
	}
	
	bool isLast() const
	{
		return _type == Enum::Invalid || _type == Enum::Eof;
	}

	CodePoint _lexeme;
	Enum      _type;
};

struct CCToken
:
	public grammar::Token
{
	enum Enum : TokenID { Invalid, Symbol, CharClassNeg, RngSep };

	static TokenID max()
	{
		return Enum::RngSep;
	}
	
	bool isLast() const
	{
		return _type == Enum::Invalid;
	}

	CodePoint _lexeme;
	Enum      _type;
};

}}
