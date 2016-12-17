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
	
	CodePoint _lexeme;
	Enum      _type;
};

struct CCToken
:
	public grammar::Token
{
	enum Enum : TokenID { Invalid, Eof, Symbol, CharClassNeg, RngSep };

	static TokenID max()
	{
		return Enum::RngSep;
	}
	
	CodePoint _lexeme;
	Enum      _type;
};

}}
