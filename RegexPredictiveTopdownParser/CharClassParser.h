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
	// cc          = [ negO ccRngLst ]      First = { symbol, ^ }
	// ccNegO      = ^                      First = { ^ }
	//             | e
	// ccRngLst    = ccRng ccRngLstT        First = { symbol }
	// ccRngLstT   = ccRng ccRngLstT        First = { symbol }
	//             | e
	// ccRng       = sybmol ccRngT          First = { symbol }
	// ccRngT      = - symbol               First = { - }
	//             | e

	using T = Token::Type;

	bool parse()
	{
		_st.init();

		return charClassNegateOpt() && charClassRangeList();
	}

private:

	bool charClassNegateOpt()
	{
		if (_st.cur()._type == T::CharClassNeg)
			return _st.m(T::CharClassNeg);
		else
			return _st.empty();
	}

	bool charClassRangeList()
	{
		return charClassRange() && charClassRangeListTail();
	}

	bool charClassRangeListTail()
	{
		if (_st.cur()._type == T::Symbol)
			return charClassRange() && charClassRangeListTail();
		else
			return _st.empty();
	}

	bool charClassRange()
	{
		return _st.m(T::Symbol) && charClassRangeTail();
	}

	bool charClassRangeTail()
	{
		if (_st.cur()._type == T::CharClassSep)
			return _st.m(T::CharClassSep) && _st.m(T::Symbol);
		else
			return _st.empty();
	}
};

}}}
