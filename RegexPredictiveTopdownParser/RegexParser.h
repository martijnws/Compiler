#pragma once

#include "ParserState.h"
#include "CharClassParser.h"
#include "Lexer.h"

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
	// factor      = symbol                        First = { symbol, [, ( }
	//             | cc
	//             | ( pattern )

	using T = Token::Type;

	// choice '|'
	bool pattern()
	{
		const std::size_t pos = _st.pos(); Token cur = _st.cur();

		// If the _st.empty string is not a valid pattern, remove the || _st.empty line
		return expr() && patternTail()
			|| (_st.retract(pos, cur), _st.empty());
	}

	bool patternTail()
	{
		const std::size_t pos = _st.pos(); Token cur = _st.cur();

		return _st.m(T::Choice) && expr() && patternTail()
			|| (_st.retract(pos, cur), _st.empty());
	}

	// concatenation
	bool expr()
	{
		return term() && exprTail();
	}

	bool exprTail()
	{
		const std::size_t pos = _st.pos(); Token cur = _st.cur();

		return term() && exprTail()
			|| (_st.retract(pos, cur), _st.empty());
	}

	// repitition '*'
	bool term()
	{
		return factor() && termTail();
	}

	bool termTail()
	{
		const std::size_t pos = _st.pos(); Token cur = _st.cur();

		return _st.m(T::ZeroToMany)
			|| (_st.retract(pos, cur), _st.empty());
	}

	// atoms
	bool factor()
	{
		const std::size_t pos = _st.pos(); Token cur = _st.cur();

		return symbol()
			|| (_st.retract(pos, cur), charClass())
			|| (_st.retract(pos, cur), _st.m(T::SubExprB) && pattern() && _st.m(T::SubExprE));
	}

	bool symbol()
	{
		return _st.m(T::Symbol);
	}

	// a '-' (aka charClassRangeSepOpt) at the beginning or end is treated as any other character
	bool charClass()
	{
		CharClassParser<Buf> body(_st.buf(), _st.cur());

		return _st.cur()._type == T::CharClassB && body.parse() && _st.m(T::CharClassE);
	}
};

}}}
