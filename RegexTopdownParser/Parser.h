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
		// load first token
		_cur = _lexer.next();

		//top down parse
		return pattern() && eof();
	}

private:

	// choice '|'
	bool pattern()
	{
		return expr() && patternTail();
	}

	bool patternTail()
	{
		return m('|') && expr() && patternTail()
			|| empty();
	}

	// concatenation
	bool expr()
	{
		return term() && exprTail();
	}

	bool exprTail()
	{
		return term() && exprTail()
			|| empty();
	}

	// repitition '*'
	bool term()
	{
		return factor() && termTail();
	}

	bool termTail()
	{
		return m('*')
			|| empty();
	}

	// atoms
	bool factor()
	{
		return symbol()
			|| m('(') && pattern() && m(')');
	}

	bool symbol()
	{
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
			|| m('\\') && m(fncIn);
	}

	bool empty() const
	{
		return true;
	}

	bool eof() const
	{
		return _cur == '\0';
	}

	bool m(char c_)
	{
		bool match = false;
		if (match = !eof() && _cur == c_)
		{
			std::cout << "match(c) " << _cur << std::endl;
			_cur = _lexer.next();
		}
		return match;
	}

	template<typename Pred>
	bool m(const Pred& pred)
	{
		bool match = false;
		if (match = !eof() && pred(_cur))
		{
			std::cout << "match(p) " << _cur << std::endl;
			_cur = _lexer.next();
		}
		return match;
	}

private:
	Lexer<Buffer> _lexer;
	char _cur;
};

