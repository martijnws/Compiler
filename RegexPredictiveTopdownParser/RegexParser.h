#pragma once

#include "ParserState.h"
#include "ParserHandler.h"
#include "CharClassParser.h"
#include "Lexer.h"
#include "Util.h"

namespace mws { namespace td { namespace pr {

template<typename BufferT>
class RegexParser
{
	ParserState<BufferT, RegexLexer<BufferT>> _st;
	ParserHandler&                            _h;

public:
	RegexParser(BufferT& buf_, Token& cur_, ParserHandler& h_)
		:
		_st(buf_, cur_), _h(h_)
	{
		
	}

	bool parse()
	{
		_st.init();

		return choice() && _st.eof() && (_h.onEof(), true);
	}

private:

	// Regex grammar:
	//
	// choice     = expr choiceT  {A;}          First = { symbol, [, ( }
	//             | e
	// choiceT =  '|' expr {A;}  choiceT        First = { |, e }
	//             | e
	// concat        = term concatT             First = { symbol, [, ( }
	// concatT    = term {A;} concatT           First = { symbol, [, ( }
	//             | e
	// term        = factor zeroToManyO         First = { symbol, [, ( }
	// zeroToManyO = * {A}                      First = { *, e }
	//             | e                             
	// factor      = symbol {A}                 First = { symbol }
	//             | charClass					First = { [ }
	//             | ( choice )                 First = { ( }

	using T = Token::Type;

	// choice '|'
	bool choice()
	{
		// If the empty string is not a valid choice, remove the empty line
		if (isIn(_st.cur(), { T::Symbol, T::CharClassB, T::SubExprB }))
			return concat() && choiceT();
		else
			return _st.empty();

	}

	bool choiceT()
	{
		if (_st.cur()._type == T::Choice)
			return _st.m(T::Choice) && concat() && (_h.onChoice(), true) && choiceT();
		else
			return _st.empty();
	}

	// concatenation
	bool concat()
	{
		return term() && concatT();
	}

	bool concatT()
	{
		if (isIn(_st.cur(), { T::Symbol, T::CharClassB, T::SubExprB }))
			return term() && (_h.onConcat(), true) && concatT();
		else
			return _st.empty();
	}

	// repitition '*'
	bool term()
	{
		return factor() && zeroToManyO();
	}

	bool zeroToManyO()
	{
		if (_st.cur()._type == T::ZeroToMany)
			return _st.m(T::ZeroToMany) && (_h.onZeroToMany(), true);
		else
			return _st.empty();
	}

	// atoms
	bool factor()
	{
		Token t = _st.cur();

		switch(_st.cur()._type)
		{
		case T::Symbol:
			return _st.m(T::Symbol) && (_h.onSymbol(t), true);
		case T::CharClassB:
			return charClass();
		case T::SubExprB:
			return _st.m(T::SubExprB) && choice() && _st.m(T::SubExprE);
		default:
			return false;
		}
	}

	bool charClass()
	{
		CharClassParser<Buf> body(_st.buf(), _st.cur(), _h);

		return body.parse();
	}
};

}}}
