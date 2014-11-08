#pragma once

namespace mws { namespace td { namespace LL1 {

struct Token
{
	enum Type 
	{
		None, Symbol, Choice, ZeroToMany, SubExprB, SubExprE, CharClassB, CharClassE, CharClassNeg, RngSep, Eof
	};

	Type _type;
	char _lexeme;
};

}}}
