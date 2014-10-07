#pragma once

#include "ParserState.h"
#include "Lexer.h"
#include "Util.h"
#include <iostream>

namespace mws { namespace td { namespace pr {

template<typename BufferT>
class CharClassParser
{
	ParserState<BufferT, CharClassLexer<BufferT>> _st;

public:
	CharClassParser(BufferT& buf_, Token& cur_)
		:
		_st(buf_, cur_)
	{
		
	}

	// charClass grammar:
	//
	// cc        = [ negO RngLst ]    First = { symbol, ^ }
	// NegO      = ^                  First = { ^ }
	//             | e
	// RngLst    = Rng RngLstT        First = { symbol }
	// RngLstT   = Rng RngLstT        First = { symbol }
	//             | e
	// Rng       = sybmol RngT        First = { symbol }
	// RngT      = - symbol           First = { - }
	//             | e

	using T = Token::Type;

	bool parse()
	{
		_st.init();

		return negateOpt() && rangeList();
	}

private:

	bool negateOpt()
	{
		if (_st.cur()._type == T::CharClassNeg)
			return _st.m(T::CharClassNeg);
		else
			return _st.empty();
	}

	bool rangeList()
	{
		return range() && rangeListTail();
	}

	bool rangeListTail()
	{
		if (_st.cur()._type == T::Symbol)
			return range() && rangeListTail();
		else
			return _st.empty();
	}

	bool range()
	{
		return _st.m(T::Symbol) && rangeTail();
	}

	bool rangeTail()
	{
		if (_st.cur()._type == T::CharClassSep)
			return _st.m(T::CharClassSep) && _st.m(T::Symbol);
		else
			return _st.empty();
	}
};

}}}
