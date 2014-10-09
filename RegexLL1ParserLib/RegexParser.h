#pragma once

#include "ParserState.h"
#include "ParserHandler.h"
#include "CharClassParser.h"
#include "Lexer.h"
#include <CommonLib/Util.h>

namespace mws { namespace td { namespace LL1 {

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

	void parse()
	{
		_st.init();

		choice(); 
		
		// failure is reported by means of exceptions. If we make it here, the full string is parsed
		assert(_st.eof());
		
		_h.onEof();
	}

private:

	// Regex grammar:
	//
	// choice     = concat choiceT              First = { symbol, [, ( }
	//             | e
	// choiceT =  '|' concat {A;}  choiceT      First = { |, e }
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
	void choice()
	{
		if (isIn(_st.cur(), { T::Symbol, T::CharClassB, T::SubExprB }))
		{
			concat(); choiceT();
		}
		else
		// If the empty string is not a valid choice, throw unconditional 
		if (!_st.eof())
		{
			throw common::Exception("Parser error: Invalid first symbol");
		}
	}

	void choiceT()
	{
		if (_st.cur()._type == T::Choice)
		{
			_st.m(T::Choice); concat(); _h.onChoice(); choiceT();
		}
	}

	// concatenation
	void concat()
	{
		term(); concatT();
	}

	void concatT()
	{
		if (isIn(_st.cur(), { T::Symbol, T::CharClassB, T::SubExprB }))
		{
			term(); _h.onConcat(); concatT();
		}
	}

	// repitition '*'
	void term()
	{
		factor(); zeroToManyO();
	}

	void zeroToManyO()
	{
		if (_st.cur()._type == T::ZeroToMany)
		{
			_st.m(T::ZeroToMany); _h.onZeroToMany();
		}
	}

	// atoms
	void factor()
	{
		Token t = _st.cur();

		switch(_st.cur()._type)
		{
		case T::Symbol:
			_st.m(T::Symbol); _h.onSymbol(t);
			break;
		case T::CharClassB:
			charClass();
			break;
		case T::SubExprB:
			_st.m(T::SubExprB); choice(); _st.m(T::SubExprE);
			break;
		default:
			throw common::Exception("Parser error: Expected factor");
		}
	}

	void charClass()
	{
		CharClassParser<Buf> cc(_st.buf(), _st.cur(), _h);

		cc.parse(); _h.onCharClass();
	}
};

}}}
