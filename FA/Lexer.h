#pragma once

#include "DFANode.h"
#include "NFA.h"
#include <Grammar/Token.h>
#include <CommonLib/Buffer.h>
#include <CommonLib/String.h>

namespace mws {

class Lexer
{
public:
	struct IPair
	{
		StringExt            regex;
		grammar::Token::Type type;
	};

    Lexer(IStreamExt& is_, const std::vector<IPair>& regexCol_);

    grammar::Token::Type next(String& lexeme_);

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

	TokenT next()
	{
		Token t;
		t._type = static_cast<TokenT::Enum>(next(t._lexeme));
		return t;
	}
};

}
