#pragma once

#include <Grammar/Token.h>

namespace mws { namespace regex {

// Eoc = End of code

struct REToken
:
	public grammar::Token
{
	enum Enum { Eof = grammar::Token::Eof, Eoc, Symbol, Choice, ZeroOrOne, ZeroToMany, OneToMany, SubExprB, SubExprE, CharClassB, CharClassE, Max };

	CodePoint _lexeme;
	Enum      _type;
};

struct CCToken
:
	public grammar::Token
{
	enum Enum { Eof = grammar::Token::Eof, Eoc, Symbol, CharClassNeg, RngSep, Max };

	CodePoint _lexeme;
	Enum      _type;
};

}}
