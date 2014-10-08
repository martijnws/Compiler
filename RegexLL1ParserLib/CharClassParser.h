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
	// charClass = [ {A;} negO RngLst ] {A;}    First = { symbol, ^ }
	// NegO      = ^ {A;}                       First = { ^ }
	//             | e
	// RngLst    = Rng RngLstT                  First = { symbol }
	// RngLstT   = Rng {A;} RngLstT             First = { symbol }
	//             | e
	// Rng       = factor RngT                  First = { symbol }
	// RngT      = - factor {A;}                First = { - }
	//             | e
	// factor    = symbol {A;}

	using T = Token::Type;

	bool parse()
	{
		return _st.m(T::CharClassB) && (_h.onCharClassBeg(), true) 
			   && 
			   negateOpt() && rangeList() 
			   && 
			   _st.m(T::CharClassE) && (_h.onCharClassEnd(), true);
	}

private:

	bool negateOpt()
	{
		if (_st.cur()._type == T::CharClassNeg)
			return _st.m(T::CharClassNeg) && (_h.onNegate(), true);
		else
			return _st.empty();
	}

	bool rangeList()
	{
		return range() && rangeListT();
	}

	bool rangeListT()
	{
		if (_st.cur()._type == T::Symbol)
			return range() && (_h.onRangeList(), true) && rangeListT();
		else
			return _st.empty();
	}

	bool range()
	{
		return factor() && rangeT();
	}

	bool rangeT()
	{
		if (_st.cur()._type == T::CharClassSep)
			return _st.m(T::CharClassSep) && factor() && (_h.onRange(), true);
		else
			return _st.empty();
	}

	bool factor()
	{
		Token t = _st.cur();
		return _st.m(T::Symbol) && (_h.onSymbol(t), true);
	}
};

}}}
