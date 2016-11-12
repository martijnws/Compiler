#pragma once

#include <Grammar/Token.h>
#include <cassert>

namespace mws { namespace td { namespace LL1 {

template<typename LexerT>
class ParserState
{
public:
 
	ParserState(LexerT& lexer_, grammar::Token& cur_)
	: 
		_lexer(lexer_), _cur(cur_)
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
		return _cur._type == grammar::Token::Eof;
	}

	grammar::Token& cur() const
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
			throw common::Exception("Lexer error");
		}

        if (fetchNext_)
        {
		    _cur = _lexer.next();
        }
	}

private:
	LexerT&         _lexer;
	grammar::Token& _cur;
};

}}}
