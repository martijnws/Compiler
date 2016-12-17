#pragma once

#include <Grammar/Token.h>

namespace mws { namespace regex {

struct REToken
:
	public grammar::Token
{
	enum Enum { Invalid = grammar::Token::Invalid, Eof, Symbol, Choice, ZeroOrOne, ZeroToMany, OneToMany, SubExprB, SubExprE, CharClassB, CharClassE };

	static grammar::Token::Type max()
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
	enum Enum { Invalid = grammar::Token::Invalid, Eof, Symbol, CharClassNeg, RngSep };

	static grammar::Token::Type max()
	{
		return Enum::RngSep;
	}
	
	CodePoint _lexeme;
	Enum      _type;
};

}}
