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
	// charClass = [ body ]                          First = { symbol, ^ }
	// body      = ^ choice {A;}                     First = { ^ }
	//           | choice                            First = { symbol }
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
		_st.m(T::CharClassB); body(); _st.m(T::CharClassE);
	}

private:

	void body()
	{
		switch(_st.cur()._type)
		{
		case T::CharClassNeg:
			_st.m(T::CharClassNeg); choice(); _h.onNegate();
			break;
		case T::Symbol:
			choice();
			break;
		default:
			throw common::Exception("Parser error: CharClass must begin with ^ or symbol");
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
