#pragma once

#include "ParserState.h"
#include "Lexer.h"
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
	// cc          = [ negO ccSet ]
	// ccSet       = - ccRngLstO 
	//             | ccRngLst ccRngSepO
	// ccNetO      = ^ 
	//             | e
	// ccRngSepO   = - 
	//             | e
	// ccRngLstO   = ccRngLst 
	//             | e
	// ccRngLst    = ccRng ccRngLstT
	// ccRngLstT   = ccRng ccRngLstT 
	//             | e
	// ccRng       = ccSym ccRngT
	// ccRngT      = - ccSym 
	//             | e
	// ccSym       = symbol
	//             | ^ 

	using T = Token::Type;

	bool parse()
	{
		_st.init();

		return charClassNegateOpt() && charClassSet();
	}

private:

	bool charClassSet()
	{
		const std::size_t pos = _st.pos(); Token cur = _st.cur();

		// a '-' (aka CharClassSep) at the beginning or end is treated as any other character
		return _st.m(T::CharClassSep) && charClassRangeListOpt()
			|| (_st.retract(pos, cur), charClassRangeList() && charClassRangeSepOpt());
	}

	bool charClassNegateOpt()
	{
		const std::size_t pos = _st.pos(); Token cur = _st.cur();

		return _st.m(T::CharClassNeg)
			|| (_st.retract(pos, cur), _st.empty());
	}

	bool charClassRangeSepOpt()
	{
		const std::size_t pos = _st.pos(); Token cur = _st.cur();

		return _st.m(T::CharClassSep)
			|| (_st.retract(pos, cur), _st.empty());
	}

	bool charClassRangeListOpt()
	{
		const std::size_t pos = _st.pos(); Token cur = _st.cur();

		return charClassRangeList()
			|| (_st.retract(pos, cur), _st.empty());
	}

	bool charClassRangeList()
	{
		return charClassRange() && charClassRangeListTail();
	}

	bool charClassRangeListTail()
	{
		const std::size_t pos = _st.pos(); Token cur = _st.cur();

		return charClassRange() && charClassRangeListTail()
			|| (_st.retract(pos, cur), _st.empty());
	}

	bool charClassRange()
	{
		return charClassSymbol() && charClassRangeTail();
	}

	bool charClassRangeTail()
	{
		const std::size_t pos = _st.pos(); Token cur = _st.cur();

		return _st.m(T::CharClassSep) && charClassSymbol()
			|| (_st.retract(pos, cur), _st.empty());
	}

	bool charClassSymbol()
	{
		const std::size_t pos = _st.pos(); Token cur = _st.cur();

		return _st.m(T::Symbol)
			|| (_st.retract(pos, cur), _st.m(T::CharClassNeg));
	}
};

}}}
