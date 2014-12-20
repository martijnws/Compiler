#pragma once

#include <Grammar/Token.h>
#include <cassert>

namespace mws { namespace td { namespace LL1 {

template<typename BufferT, typename LexerT>
class ParserState
{
public:
 
	ParserState(BufferT& buf_, grammar::Token& cur_)
		: _buf(buf_), _cur(cur_), _lexer(buf_)
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

	BufferT& buf()
	{
		return _buf;
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
	BufferT&        _buf;
	grammar::Token& _cur;
	LexerT          _lexer;
};

}}}
