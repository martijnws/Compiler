#pragma once

#include "ParserState.h"
#include "CharClassParser.h"
#include "Lexer.h"
#include "Util.h"

namespace mws { namespace td { namespace pr {

template<typename BufferT>
class RegexParser
{
	ParserState<BufferT, RegexLexer<BufferT>> _st;

public:
	RegexParser(BufferT& buf_, Token& cur_)
		:
		_st(buf_, cur_)
	{
		
	}

	bool parse()
	{
		_st.init();

		return pattern() && _st.eof();
	}

private:

	// Regex grammar:
	//
	// pattern     = expr patternTail              First = { symbol, [, ( }
	//             | e
	// patternTail =  '|' expr  patternTail        First = { |, e }
	//             | e
	// expr        = term exprTail                 First = { symbol, [, ( }
	// exprTail    = term exprTail                 First = { symbol, [, ( }
	//             | e
	// term        = factor termTail               First = { symbol, [, ( }
	// termTail    = *                             First = { *, e }
	//             | e                             
	// factor      = symbol                        First = { symbol }
	//             | cc							   First = { [ }
	//             | ( pattern )                   First = { ( }

	using T = Token::Type;

	// choice '|'
	bool pattern()
	{
		// If the empty string is not a valid pattern, remove the empty line
		if (isIn(_st.cur(), { T::Symbol, T::CharClassB, T::SubExprB }))
			return expr() && patternTail();
		else
			return _st.empty();

	}

	bool patternTail()
	{
		if (_st.cur()._type == T::Choice)
			return _st.m(T::Choice) && expr() && patternTail();
		else
			return _st.empty();
	}

	// concatenation
	bool expr()
	{
		return term() && exprTail();
	}

	bool exprTail()
	{
		if (isIn(_st.cur(), { T::Symbol, T::CharClassB, T::SubExprB }))
			return term() && exprTail();
		else
			return _st.empty();
	}

	// repitition '*'
	bool term()
	{
		return factor() && termTail();
	}

	bool termTail()
	{
		if (_st.cur()._type == T::ZeroToMany)
			return _st.m(T::ZeroToMany);
		else
			return _st.empty();
	}

	// atoms
	bool factor()
	{
		switch(_st.cur()._type)
		{
		case T::Symbol:
			return _st.m(T::Symbol);
		case T::CharClassB:
			return charClass();
		case T::SubExprB:
			return _st.m(T::SubExprB) && pattern() && _st.m(T::SubExprE);
		default:
			return false;
		}
	}

	bool charClass()
	{
		CharClassParser<Buf> body(_st.buf(), _st.cur());

		return _st.cur()._type == T::CharClassB && body.parse() && _st.m(T::CharClassE);
	}
};

}}}
