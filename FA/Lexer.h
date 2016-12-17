#pragma once

#include "DFANode.h"
#include "NFA.h"
#include <Grammar/Token.h>
#include <CommonLib/Buffer.h>
#include <CommonLib/String.h>
#include <CommonLib/TokenID.h>

namespace mws {

class Lexer
{
public:
	struct IPair
	{
		StringExt regex;
		TokenID   type;
	};

    Lexer(IStreamExt& is_, const std::vector<IPair>& regexCol_);

    bool next(String& lexeme_, TokenID& type_);

private:
    common::BufferExt _buf;
    DFANode*          _dfa;
    bool              _eof;
};


template<typename TokenT>
class LexerT
	:
	public Lexer
{
public:
	using Lexer::Lexer;
	using Lexer::next;

	using Token = TokenT;

	TokenT next()
	{
		Token t;
		auto type = InvalidTokenID;
		if (next(t._lexeme, type))
		{
			assert(type != InvalidTokenID);
			assert(type <= Token::max());
			t._type = static_cast<Token::Enum>(type);
		}
		else
		{
			t._type = Token::Invalid;
		}

		return t;
	}
};

}
