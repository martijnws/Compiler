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

/*
	bool eof() const
	{
		return _cur._type == Token::Enum::Eof;
	}

	bool invalid() const
	{
		return _cur._type == Token::Enum::Invalid;
	}
*/
	const auto& cur() const
	{
		return _cur;
	}

    void m(TokenID type_)
	{
        m(type_, true);
    }

	void m(TokenID type_, bool fetchNext_)
	{
		const auto type = static_cast<Token::Enum>(type_);

		if (_cur._type != type)
		{
			mws::StringStream s;
			s << "Parser Error: Expected = " << type_ << ", Actual = " << _cur._type;
			throw common::Exception(s.str());
		}

        if (fetchNext_)
        {
		    _cur = _lexer.next();
        }
	}

private:
	LexerT&  _lexer;
	Token    _cur;
};

}}
