#pragma once

#include "ParserState.h"
#include "ParserHandler.h"
#include "Lexer.h"
#include <CommonLib/Util.h>
#include <iostream>

namespace mws { namespace td { namespace LL1 {

template<typename BufferT>
class CharClassParser
{
	ParserState<BufferT, CharClassLexer<BufferT>> _st;
	ParserHandler&                                _h;

public:
	CharClassParser(BufferT& buf_, Token& cur_, ParserHandler& h_)
		:
		_st(buf_, cur_), _h(h_)
	{
		
	}

	// charClass grammar:
	//
	// charClass = [ negO RngLst  {A (if negO);} ]   First = { symbol, ^ }
	// negO      = ^ {A;}                            First = { ^ }
	//             | e
	// choice    = Rng choiceT                       First = { symbol }
	// choiceT   = Rng {A;} choiceT                  First = { symbol }
	//             | e
	// Rng       = option RngT                       First = { symbol }
	// RngT      = - option {A;}                     First = { - }
	//             | e
	// option    = symbol {A;}

	using T = Token::Type;

	void parse()
	{
		bool isNegate = false;

		_st.m(T::CharClassB); negateOpt(isNegate); choice(); _st.m(T::CharClassE);

		if (isNegate)
		{
			_h.onNegate();
		}
	}

private:

	void negateOpt(bool& isNegate)
	{
		if (isNegate = _st.cur()._type == T::CharClassNeg)
		{
			_st.m(T::CharClassNeg);
		}
	}

	// concatenation in charClass means choice
	void choice()
	{
		range(); choiceT();
	}

	void choiceT()
	{
		if (_st.cur()._type == T::Symbol)
		{
			range(); _h.onChoice(); choiceT();
		}
	}

	void range()
	{
		option(); rangeT();
	}

	void rangeT()
	{
		if (_st.cur()._type == T::CharClassSep)
		{
			_st.m(T::CharClassSep); option(); _h.onRange();
		}
	}

	void option()
	{
		Token t = _st.cur();
		_st.m(T::Symbol); _h.onSymbol(t);
	}
};

}}}
