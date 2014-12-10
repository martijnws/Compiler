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

struct GrammarSymbol;

using onTokenEvt = void (ParserHandler::*)(const Token&);
using onEvt      = void (ParserHandler::*)();
using action     = std::function<void(const Token& t_, ParserHandler& h_, TokenStore& store_)>;
using parse      = std::function<void(const GrammarSymbol& startSymbol_, common::Buffer<256>& buf_, Token& t_, ParserHandler& h_)>;

struct GrammarSymbol
{
	bool     _isTerminal;
    bool     _isSubGrammarStartSymbol;
    bool     _fetchNext;
	uint8_t  _type;
	action   _action;
    parse    _parse;
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

enum NonTerminal { Start, Choice, ChoiceT, Concat, ConcatT, Term, QuantifierO, ZeroOrOne, ZeroToMany, OneToMany, Factor, CharClass, RngConcat, RngConcatT, Rng, RngT, Option };

GrammarSymbol n(uint32_t type_);

bool init();

const Production& expand(NonTerminal nt_, Token::Type t_);

template< template<typename> class LexerT>
class ParserDriver
{
public:
	using Buf = common::Buffer<256>;

private:
	ParserState<Buf, LexerT<Buf>> _st;
	ParserHandler&                _h;

public:
	ParserDriver(Buf& buf_, Token& cur_, ParserHandler& h_)
		:
		_st(buf_, cur_), _h(h_)
	{
		
	}

	void parse();
    void parse(const GrammarSymbol& startSymbol_);
};

template< template<typename> class LexerT>
void ParserDriver<LexerT>::parse()
{
    parse(n(NonTerminal::Start));
}

template< template<typename> class LexerT>
void ParserDriver<LexerT>::parse(const GrammarSymbol& startSymbol_)
{
	_st.init();

	typedef std::pair<GrammarSymbol, bool> GSEntry;

	TokenStore store;

	std::vector<GSEntry> stack;
	stack.push_back(std::make_pair(startSymbol_, false));

	while (!stack.empty())
	{
		GSEntry& entry = stack.back();
		const GrammarSymbol& gs = entry.first;

        Token tokCur = _st.cur();

        if (gs._isSubGrammarStartSymbol)
        {
            // in sub parse, this startSymbol becomes the toplevel startsymbol.
            GrammarSymbol startSymbol(gs);
            // To prevent infinite recursion.
            startSymbol._isSubGrammarStartSymbol = false;
            gs._parse(startSymbol, _st.buf(), _st.cur(), _h);

            // Do not execute action. It is already executed in sub parse
			stack.pop_back();
			continue;
        }

		if (gs._isTerminal)
		{
			// match
			_st.m(static_cast<Token::Type>(gs._type), gs._fetchNext);
            // mark entry as expanded
		    entry.second = true;

		}

		// is entry expanded?
		if (entry.second)
		{
			gs._action(tokCur, _h, store);
			stack.pop_back();
			continue;
		}

		assert(!gs._isTerminal);
		// mark entry as expanded
		entry.second = true;

		// expand
		const Production& prod = expand(static_cast<NonTerminal>(gs._type), tokCur._type);
		for (int i = static_cast<int>(prod._gsList.size()) - 1; i >= 0; --i)
		{
			const GrammarSymbol& gs = prod._gsList[i];
			stack.push_back(std::make_pair(gs, false));
		}
	}

	//assert(_st.eof());
}


}}}