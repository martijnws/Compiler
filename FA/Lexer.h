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
	using Buffer = common::BufferExt;

	struct TokenInfo
	{
		TokenInfo() = default;

		TokenInfo(const StringExt& regex_, TokenID type_, bool skip_ = false)
		:
			regex(regex_), type(type_), skip(skip_) 
		{

		}

		StringExt regex;
		TokenID   type;
		bool      skip = false;
	};

	using TokenInfoCol = std::vector<TokenInfo>;

    Lexer(IStreamExt& is_, const TokenInfoCol& tokenInfoCol_);

    bool next(String& lexeme_, TokenID& type_);
    bool next(String& lexeme_, TokenID& type_, bool& skip_);

private:
    Buffer       _buf;
	TokenInfoCol _tokenInfoCol;
    DFANode*     _dfa = nullptr;
    bool         _eof = false;
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
