#pragma once

#include "Buffer.h"
#include "Lexer.h"
#include <algorithm>
#include <functional>
#include <cassert>
#include <iostream>

namespace mws { namespace td { namespace pr {

// Notes on grammar
//
// A list containing zero or more items
//
// List     = Item List | empty
//
// A list containing at least 1 item
//
// List     = Item ListTail
// ListTail = Item ListTail | e
//
//  An optional item
//
// OptItem  = Item | e
//
//
// Whenever a non-terminal has more than 1 possible production we track the lexer position so that we can retry the next production if one fails
//

class Parser
{
public:
	Parser(std::istream& is_)
		:
		_buf(is_),
		_regexLexer(_buf),
		_charClassLexer(_buf),
		_lexer(&_regexLexer)
	{
		_cur = { Token::Type::None, '\0' };
	}

	bool parse()
	{
		try
		{
			//load first token
			_cur = _lexer->next();

			//top down parse
			return pattern() && eof();
		}
		catch(const Exception& e)
		{
			std::cout << "Exception: " << e.what() << std::endl;
			return false;
		}
	}

private:

	// Regex grammar:
	//
	// pattern     = expr patternTail              First = { symbol, [, ( }
	//             | e
	// patternTail =  '|' expr  patternTail        First = { |, e }
	//             | e
	// expr        = term exprTail                 First = { symbol, [, ( }
	// exprTail    = term exprTail                 First = { symbol, [, ( }
	//             | e
	// term        = factor termTail               First = { symbol, [, ( }
	// termTail    = *                             First = { *, e }
	//             | e                             
	// factor      = symbol                        First = { symbol, [, ( }
	//             | cc
	//             | ( pattern )
	// symbol      = NotIn: special                First = { symbol }
	//             | '\'In: special
	// special     = '|' * (  ) [ ] \

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
	// ccSym       = notSpecial
	//             | ^ 
	//             | \ special
	// special     = ] | - | ^
	// notSpecial  = NotIn: special

	using T = Token::Type;

	// choice '|'
	bool pattern()
	{
		const std::size_t pos = _lexer->pos(); Token cur = _cur;

		// If the empty string is not a valid pattern, remove the || empty line
		return expr() && patternTail()
			|| (retract(pos, cur), empty());
	}

	bool patternTail()
	{
		const std::size_t pos = _lexer->pos(); Token cur = _cur;

		return m(T::Choice) && expr() && patternTail()
			|| (retract(pos, cur), empty());
	}

	// concatenation
	bool expr()
	{
		return term() && exprTail();
	}

	bool exprTail()
	{
		const std::size_t pos = _lexer->pos(); Token cur = _cur;

		return term() && exprTail()
			|| (retract(pos, cur), empty());
	}

	// repitition '*'
	bool term()
	{
		return factor() && termTail();
	}

	bool termTail()
	{
		const std::size_t pos = _lexer->pos(); Token cur = _cur;

		return m(T::ZeroToMany)
			|| (retract(pos, cur), empty());
	}

	// atoms
	bool factor()
	{
		const std::size_t pos = _lexer->pos(); Token cur = _cur;

		return symbol()
			|| (retract(pos, cur), charClass())
			|| (retract(pos, cur), m(T::SubExprB) && pattern() && m(T::SubExprE));
	}

	bool symbol()
	{
		return m(T::Symbol);
	}

	// a '-' (aka charClassRangeSepOpt) at the beginning or end is treated as any other character
	bool charClass()
	{
		assert(_lexer == &_regexLexer);

		if (!m(T::CharClassB))
			return false;

		_lexer = &_charClassLexer;

		bool res = charClassNegateOpt() && charClassSet();

		_lexer = &_regexLexer;

		if (res)
			res = m(T::CharClassE);

		return res;
	}

	bool charClassSet()
	{
		const std::size_t pos = _lexer->pos(); Token cur = _cur;

		return m(T::CharClassSep) && charClassRangeListOpt()
			|| (retract(pos, cur), charClassRangeList() && charClassRangeSepOpt());
	}

	bool charClassNegateOpt()
	{
		const std::size_t pos = _lexer->pos(); Token cur = _cur;

		return m(T::CharClassNeg)
			|| (retract(pos, cur), empty());
	}

	bool charClassRangeSepOpt()
	{
		const std::size_t pos = _lexer->pos(); Token cur = _cur;

		return m(T::CharClassSep)
			|| (retract(pos, cur), empty());
	}

	bool charClassRangeListOpt()
	{
		const std::size_t pos = _lexer->pos(); Token cur = _cur;

		return charClassRangeList()
			|| (retract(pos, cur), empty());
	}

	bool charClassRangeList()
	{
		return charClassRange() && charClassRangeListTail();
	}

	bool charClassRangeListTail()
	{
		const std::size_t pos = _lexer->pos(); Token cur = _cur;

		return charClassRange() && charClassRangeListTail()
			|| (retract(pos, cur), empty());
	}

	bool charClassRange()
	{
		return charClassSymbol() && charClassRangeTail();
	}

	bool charClassRangeTail()
	{
		const std::size_t pos = _lexer->pos(); Token cur = _cur;

		return m(T::CharClassSep) && charClassSymbol()
			|| (retract(pos, cur), empty());
	}

	bool charClassSymbol()
	{
		const std::size_t pos = _lexer->pos(); Token cur = _cur;

		return m(T::Symbol)
			|| (retract(pos, cur), m(T::CharClassNeg));
	}

	bool empty() const
	{
		return true;
	}

	bool eof() const
	{
		return _cur._type == Token::Type::Eof;
	}

	void retract(std::size_t pos_, const Token& t_)
	{
		_lexer->retract(_lexer->pos() - pos_);
		_cur = t_;
	}

	//Note: match does not assert/throw on _lexer->pos() == eof() because this parser allows for backtracking. If the current production fails, there may
	//be an empty() production that makes the parse succeed without further matching.
	//If no empty() production exists, a false result propagates back up to the toplevel invocation.

	bool m(Token::Type type_)
	{
		bool match = false;
		if (!eof())
		{
			if (match = _cur._type == type_)
			{
				//std::cout << "match(c) " << _cur << std::endl;
			}

			_cur = _lexer->next();
		}

		return match;
	}

private:
	typedef Buffer<256> Buf;

	Buf _buf;
	RegexLexer<Buf> _regexLexer;
	CharClassLexer<Buf> _charClassLexer;
	ILexer* _lexer;
	Token _cur;
};

}}}
