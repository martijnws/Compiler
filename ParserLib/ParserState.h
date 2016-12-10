#pragma once

#include <Grammar/Token.h>
#include <cassert>

namespace mws { namespace td {

template<typename LexerT>
class ParserState
{
	using Token = typename LexerT::Token;

public:
 
	ParserState(LexerT& lexer_)
	: 
		_lexer(lexer_)
	{
		
	}

	void init()
	{
		next();
	}

	void next()
	{
		//load first token
		_cur = _lexer.next();
	}

	bool eof() const
	{
		return _cur._type == Token::Eof;
	}

	bool eoc() const
	{
		return _cur._type == Token::Eoc;
	}

	auto& cur() const
	{
		return _cur;
	}

    void m(grammar::Token::Type type_)
	{
        m(type_, true);
    }

	void m(grammar::Token::Type type_, bool fetchNext_)
	{
		if (eof() || _cur._type != type_)
		{
			throw common::Exception(_C("Lexer error"));
		}

        if (fetchNext_)
        {
		    _cur = _lexer.next();
        }
	}

	void retractLast()
	{
		_lexer.retractLast();
	}

private:
	LexerT&  _lexer;
	Token    _cur;
};

}}
