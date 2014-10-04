#pragma once

#include "Buffer.h"
#include "Lexer.h"
#include <algorithm>
#include <cassert>
#include <iostream>

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
		_lexer(is_)
	{

	}

	bool parse()
	{
		//top down parse
		return pattern() && eof();
	}

private:

	// choice '|'
	bool pattern()
	{
		const std::size_t count = _lexer.count();

		// If the empty string is not a valid pattern, remove the || empty line
		return expr() && patternTail()
			|| (retract(count), empty());
	}

	bool patternTail()
	{
		const std::size_t count = _lexer.count();

		return m('|') && expr() && patternTail()
			|| (retract(count), empty());
	}

	// concatenation
	bool expr()
	{
		return term() && exprTail();
	}

	bool exprTail()
	{
		const std::size_t count = _lexer.count();

		return term() && exprTail()
			|| (retract(count), empty());
	}

	// repitition '*'
	bool term()
	{
		return factor() && termOpt();
	}

	bool termOpt()
	{
		const std::size_t count = _lexer.count();

		return m('*')
			|| (retract(count), empty());
	}

	// atoms
	bool factor()
	{
		const std::size_t count = _lexer.count();

		return symbol()
			|| (retract(count), charClass())
			|| (retract(count), m('(') && pattern() && m(')'));
	}

	bool symbol()
	{
		const std::size_t count = _lexer.count();

		// raw string syntax: R"delimOfChoice(rawChars*)delimOfChoice"
		static const std::string special = { R"R(|*()[]\)R" };

		return m(isNotIn(special))
			|| (retract(count), m('\\') && m(isIn(special)));
	}

	// charClass grammar:
	//
	// cc         = [ negO ccSet ]
	// ccSet      = - ccRngLstO | ccRngLst ccRngSepO
	// ccNetO     = ^ | e
	// ccRngSepO  = - | e
	// ccRngLstO  = ccRngLst | e
	// ccRngLst   = ccRng ccRngLstT
	// ccRngLstT  = ccRng ccRngLstT | e
	// ccRng      = ccSym ccRngT
	// ccRngT     = - ccSym | e
	// ccSym      = notSpecial | ^ | \ special
	// special    = ] | - | ^
	// notSpecial = complement(special)

	// a '-' (aka charClassRangeSepOpt) at the beginning or end is treated as any other character
	bool charClass()
	{
		return m('[') && charClassNegateOpt() && charClassSet() && m(']');
	}
	
	bool charClassSet()
	{
		const std::size_t count = _lexer.count();

		return m('-') && charClassRangeListOpt()
			|| (retract(count), charClassRangeList() && charClassRangeSepOpt());
	}

	bool charClassNegateOpt()
	{
		const std::size_t count = _lexer.count();

		return m('^')
			|| (retract(count), empty());
	}

	bool charClassRangeSepOpt()
	{
		const std::size_t count = _lexer.count();

		return m('-')
			|| (retract(count), empty());
	}

	bool charClassRangeListOpt()
	{
		const std::size_t count = _lexer.count();

		return charClassRangeList()
			|| (retract(count), empty());
	}

	bool charClassRangeList()
	{
		return charClassRange() && charClassRangeListTail();
	}

	bool charClassRangeListTail()
	{
		const std::size_t count = _lexer.count();

		return charClassRange() && charClassRangeListTail()
			|| (retract(count), empty());
	}

	bool charClassRange()
	{
		return charClassSymbol() && charClassRangeTail();
	}

	bool charClassRangeTail()
	{
		const std::size_t count = _lexer.count();

		return m('-') && charClassSymbol()
			|| (retract(count), empty());
	}

	bool charClassSymbol()
	{
		const std::size_t count = _lexer.count();

		// Note: the '^' symbol is in the special list yet we override this decision in the grammar below.
		// This is done because '^' may be prefixed with or without escape. Its both special and not special
		static const std::string special = { R"R(-]^\)R" };

		return m(isNotIn(special))
			|| (retract(count), m('^'))
			|| (retract(count), m('\\') && m(isIn(special)));
	}

	bool empty() const
	{
		return true;
	}

	bool eof() const
	{
		return _lexer.cur() == '\0';
	}

	// Some awesome new C++11 functional programming features! A compact way of building commands/functors with state.
	// 
	// isNotIn is a factory function that returns a lambda expression. 
	std::function<bool(char)> isNotIn(const std::string& special)
	{
		// The & indicates that the lamda expression captures variables in scope. Thus the lamda expr returned here captures the 'special' variable.
		return [&](char cur_)
		{
			// the implementation uses an stl algo that in turn expects a functor. Again a lamba expr is used. This lamda also captures variables
			// in scope. The 'cur_' variable in this case
			return std::none_of(special.begin(), special.end(), [&](char c_){ return cur_ == c_; });
		};
	}

	std::function<bool(char)> isIn(const std::string& special)
	{
		return [&](char cur_)
		{
			return std::any_of(special.begin(), special.end(), [&](char c_){ return c_ == cur_; });
		};
	}

	void retract(std::size_t count_)
	{
		_lexer.retract(_lexer.count() - count_);
	}

	//Note: match does not assert/throw on _lexer.pos() == eof() because this parser allows for backtracking. If the current production fails, there may
	//be an empty() production that makes the parse succeed without further matching.
	//If no empty() production exists, a false result propagates back up to the toplevel invocation.

	bool m(char c_)
	{
		bool match = false;
		if (!eof())
		{
			if (match = _lexer.cur() == c_)
			{
				//std::cout << "match(c) " << _lexer.cur() << std::endl;
			}

			_lexer.next();
		}
		
		return match;
	}

	bool m(const std::function<bool (char)>& pred)
	{
		bool match = false;
		if (!eof())
		{
			if (match = pred(_lexer.cur()))
			{
				//std::cout << "match(p) " << _lexer.cur() << std::endl;
			}

			_lexer.next();
		}

		return match;
	}

private:
	Lexer<Buffer> _lexer;
};

