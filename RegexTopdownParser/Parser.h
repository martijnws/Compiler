#pragma once

#include "Buffer.h"
#include "Lexer.h"
#include <algorithm>
#include <cassert>
#include <iostream>

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
		return factor() && termTail();
	}

	bool termTail()
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
			|| (retract(count), m('(') && pattern() && m(')'));
	}

	bool symbol()
	{
		const std::size_t count = _lexer.count();

		// statics are captured automatically by lambas (no need for [&] capture instruction)
		static const std::string special = { "|*()[]" };

		auto fncNotIn = [](char cur_)
		{
			return std::none_of(special.begin(), special.end(), [&](char c_){ return cur_ == c_; });
		};

		auto fncIn = [](char cur_)
		{
			return std::any_of(special.begin(), special.end(), [&](char c_){ return c_ == cur_; });
		};

		return m(fncNotIn)
			|| (retract(count), m('\\') && m(fncIn));
	}

	bool empty() const
	{
		return true;
	}

	bool eof() const
	{
		return _lexer.cur() == '\0';
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
				std::cout << "match(c) " << _lexer.cur() << std::endl;
			}

			_lexer.next();
		}
		
		return match;
	}

	template<typename Pred>
	bool m(const Pred& pred)
	{
		bool match = false;
		if (!eof())
		{
			if (match = pred(_lexer.cur()))
			{
				std::cout << "match(p) " << _lexer.cur() << std::endl;
			}

			_lexer.next();
		}

		return match;
	}

private:
	Lexer<Buffer> _lexer;
};

