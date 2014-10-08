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

	bool parse()
	{
		bool isNegate = false;

		bool res = _st.m(T::CharClassB) && negateOpt(isNegate) && choice() && _st.m(T::CharClassE);

		if (res && isNegate)
		{
			_h.onNegate();
		}

		return res;
	}

private:

	bool negateOpt(bool& isNegate)
	{
		if (isNegate = _st.cur()._type == T::CharClassNeg)
			return _st.m(T::CharClassNeg);
		else
			return _st.empty();
	}

	// concatenation in charClass means choice
	bool choice()
	{
		return range() && choiceT();
	}

	bool choiceT()
	{
		if (_st.cur()._type == T::Symbol)
			return range() && (_h.onChoice(), true) && choiceT();
		else
			return _st.empty();
	}

	bool range()
	{
		return option() && rangeT();
	}

	bool rangeT()
	{
		if (_st.cur()._type == T::CharClassSep)
			return _st.m(T::CharClassSep) && option() && (_h.onRange(), true);
		else
			return _st.empty();
	}

	bool option()
	{
		Token t = _st.cur();
		return _st.m(T::Symbol) && (_h.onSymbol(t), true);
	}
};

}}}
