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
	// cc          = [ negO ccSet ]         First = { symbol, ^, - }
	// ccSet       = - ccRngLstO            First = { - }
	//             | ccRngLst ccRngSepO     First = { symbol, ^ }
	// ccNegO      = ^                      First = { ^ }
	//             | e
	// ccRngSepO   = -                      First = { - }
	//             | e
	// ccRngLstO   = ccRngLst               First = { symbol, ^ }
	//             | e
	// ccRngLst    = ccRng ccRngLstT        First = { symbol, ^ }
	// ccRngLstT   = ccRng ccRngLstT        First = { symbol, ^ }
	//             | e
	// ccRng       = ccSym ccRngT           First = { symbol, ^ }
	// ccRngT      = - ccSym                First = { - }
	//             | e
	// ccSym       = symbol                 First = { symbol }
	//             | ^                      First = { ^ }

	using T = Token::Type;

	bool parse()
	{
		_st.init();

		return charClassNegateOpt() && charClassSet();
	}

private:

	bool charClassSet()
	{
		// a '-' (aka CharClassSep) at the beginning or end is treated as any other character

		if (isIn(_st.cur(), {T::Symbol, T::CharClassNeg}))
			return charClassRangeList() && charClassRangeSepOpt();
		else
		if (_st.cur()._type == T::CharClassSep)
			return _st.m(T::CharClassSep) && charClassRangeListOpt();
		else
			return false;
	}

	bool charClassNegateOpt()
	{
		if (_st.cur()._type == T::CharClassNeg)
			return _st.m(T::CharClassNeg);
		else
			return _st.empty();
	}

	bool charClassRangeSepOpt()
	{
		if (_st.cur()._type == T::CharClassSep)
			return _st.m(T::CharClassSep);
		else
			return _st.empty();
	}

	bool charClassRangeListOpt()
	{
		if (isIn(_st.cur(), {T::Symbol, T::CharClassNeg}))
			return charClassRangeList();
		else
			return _st.empty();
	}

	bool charClassRangeList()
	{
		return charClassRange() && charClassRangeListTail();
	}

	bool charClassRangeListTail()
	{
		if (isIn(_st.cur(), {T::Symbol, T::CharClassNeg}))
			return charClassRange() && charClassRangeListTail();
		else
			return _st.empty();
	}

	bool charClassRange()
	{
		return charClassSymbol() && charClassRangeTail();
	}

	bool charClassRangeTail()
	{
		if (_st.cur()._type == T::CharClassSep)
			return _st.m(T::CharClassSep) && charClassSymbol();
		else
			return _st.empty();
	}

	bool charClassSymbol()
	{
		switch(_st.cur()._type)
		{
		case T::Symbol:
			return _st.m(T::Symbol);
		case T::CharClassNeg:
			return _st.m(T::CharClassNeg);
		default:
			return false;
		}
	}
};

}}}
