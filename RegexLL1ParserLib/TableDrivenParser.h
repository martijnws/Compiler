#pragma once

#pragma once

#include "ParserHandler.h"
#include "Token.h"
#include "ParserState.h"
#include "ParserHandler.h"
#include "Lexer.h"
#include <CommonLib/Util.h>
#include <functional>
#include <initializer_list>
#include <vector>
#include <set>
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
	bool     _isTerminal;
	uint8_t  _type;
	action   _action;
};

struct Production
{
	Production(const std::initializer_list<GrammarSymbol>& gsList_)
		: _gsList(gsList_.begin(), gsList_.end()), _derivesEmpty(false)
	{
		
	}

	const std::vector<GrammarSymbol> _gsList;
	std::set<uint8_t>                _first;
	bool                             _derivesEmpty;
};

enum NonTerminal { Choice, ChoiceT, Concat, ConcatT, Term, ZeroToManyO, Factor, CharClass };

bool init();

const Production& expand(NonTerminal nt_, Token::Type t_);

class ParserDriver
{
	using Buf = common::Buffer<256>;

	ParserState<Buf, RegexLexer<Buf>> _st;
	ParserHandler&                    _h;

public:
	ParserDriver(Buf& buf_, Token& cur_, ParserHandler& h_)
		:
		_st(buf_, cur_), _h(h_)
	{
		
	}

	void parse();
};

}}}