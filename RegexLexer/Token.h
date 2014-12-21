#pragma once

#include <Grammar/Token.h>

namespace mws { namespace regex {

struct Token : public grammar::Token 
{
	enum Enum { Eof = grammar::Token::Eof, Symbol, Choice, ZeroOrOne, ZeroToMany, OneToMany, SubExprB, SubExprE, CharClassB, CharClassE, CharClassNeg, RngSep, Max };
};

}}
