#pragma once

#pragma once

#include "ParserHandler.h"
#include "Token.h"
#include <functional>
#include <initializer_list>
#include <cstdint>
#include <cassert>

namespace mws { namespace td { namespace LL1 {

class TokenStore
{
public:
	Token get(uint8_t index_) const
	{
		return _tokens[index_];
	}

	Token pop(uint8_t index_)
	{
		Token t = get(index_);
		_tokens[index_]._lexeme = 0;
		_tokens[index_]._type = Token::Type::None;
		return t;
	}

	void  save(uint8_t index_, const Token& t_)
	{
		assert(_tokens[index_]._type != Token::Type::None);
		_tokens[index_] = t_;
	}

private:
	Token _tokens[256];
};


using onTokenEvt = void (ParserHandler::*)(const Token&);
using onEvt      = void (ParserHandler::*)();
using action     = std::function<void(const Token& t_, ParserHandler& h_, TokenStore& store_)>;

struct GrammarSymbol
{
	uint32_t _type;
	action   _action;
};

struct Production
{
	Production(const std::initializer_list<GrammarSymbol>& gsList_)
		: _gsList(gsList_)
	{
	
	}

	const std::initializer_list<GrammarSymbol> _gsList;
};

enum NonTerminal { Choice = Token::Type::Eof + 1, ChoiceT, Concat, ConcatT, Term, ZeroToManyO, Factor, Empty, CharClass };

bool init();

const Production& expand(NonTerminal nt_, Token::Type t_);

}}}