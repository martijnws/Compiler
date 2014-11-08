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
	// body      = ^ rngConcat {A;}                     First = { ^ }
	//           | rngConcat                            First = { symbol }
	// rngConcat    = Rng rngConcatT                       First = { symbol }
	// rngConcatT   = Rng {A;} rngConcatT                  First = { symbol }
	//             | e
	// Rng       = option RngT                       First = { symbol }
	// RngT      = - option {A;}                     First = { - }
	//             | e
	// option    = symbol {A;}

	using T = Token::Type;

	void parse()
	{
		_st.init();

		body();
	}

private:

	void body()
	{
		switch(_st.cur()._type)
		{
		case T::CharClassNeg:
			_st.m(T::CharClassNeg); rngConcat(); _h.onNegate();
			break;
		case T::Symbol:
			rngConcat();
			break;
		default:
			throw common::Exception("Parser error: CharClass must begin with ^ or symbol");
		}
	}

	void rngConcat()
	{
		rng(); rngConcatT();
	}

	void rngConcatT()
	{
		if (_st.cur()._type == T::Symbol)
		{
			rng(); _h.onRngConcat(); rngConcatT();
		}
	}

	void rng()
	{
		option(); rngT();
	}

	void rngT()
	{
		if (_st.cur()._type == T::RngSep)
		{
			_st.m(T::RngSep); option(); _h.onRng();
		}
	}

	void option()
	{
		Token t = _st.cur();
		_st.m(T::Symbol); _h.onSymbol(t);
	}
};

}}}
