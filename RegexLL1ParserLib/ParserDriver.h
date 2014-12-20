#pragma once

#include "ParserHandler.h"
#include "ParserState.h"
#include "ParserTable.h"
#include "Lexer.h"
#include <Grammar/Grammar.h>
#include <Grammar/TokenStore.h>

namespace mws { namespace td { namespace LL1 {

template< template<typename> class LexerT>
class ParserDriver
{
	ParserState<common::Buffer, LexerT<common::Buffer>> _st;

public:
	ParserDriver(common::Buffer& buf_, grammar::Token& cur_)
		:
		_st(buf_, cur_)
	{
		
	}

    void parse(grammar::Handler& h_, const grammar::Grammar& grammar_);
    void parse(grammar::Handler& h_, const grammar::Grammar& grammar_, const grammar::GrammarSymbol& startSymbol_);
};

template< template<typename> class LexerT>
void ParserDriver<LexerT>::parse(grammar::Handler& h_, const grammar::Grammar& grammar_)
{
    // type = 0 indicates this GrammarSymbol is an instance of NT at grammar[0]. That is, it is an instance of Start
    grammar::GrammarSymbol startSymbol = grammar::n(0);
    parse(h_, grammar_, startSymbol);
}

template< template<typename> class LexerT>
void ParserDriver<LexerT>::parse(grammar::Handler& h_, const grammar::Grammar& grammar_, const grammar::GrammarSymbol& startSymbol_)
{
    const ParserTable& parserTable = ParserTable::Cache::get(&grammar_);

	_st.init();

	using GSEntry = std::pair<grammar::GrammarSymbol, bool>;

	grammar::TokenStore store;

	std::vector<GSEntry> stack;
	stack.push_back(std::make_pair(startSymbol_, false));

	while (!stack.empty())
	{
		auto& entry = stack.back();
		const auto& gs = entry.first;

        // copy by value since later on _st.cur() will change
        const auto tokCur = _st.cur();

        if (gs.isSubGrammarStartSymbol())
        {
            // in sub parse, this startSymbol becomes the toplevel startsymbol.
            grammar::GrammarSymbol startSymbol(gs);
            // To prevent infinite recursion.
            startSymbol._parse = nullptr;
            assert(!startSymbol.isSubGrammarStartSymbol());

            gs._parse(h_, grammar_, startSymbol, _st.buf(), _st.cur());

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
			gs._action(h_, tokCur, store);
			stack.pop_back();
			continue;
		}

		assert(!gs._isTerminal);
		// mark entry as expanded
		entry.second = true;

		// expand
		const auto& prod = parserTable.expand(grammar_, gs._type, tokCur._type);
        // signed i used to detect >= condition
		for (int i = static_cast<int>(prod._gsList.size()) - 1; i >= 0; --i)
	    {
			stack.push_back(std::make_pair(prod._gsList[i], false));
		}
	}

	//assert(_st.eof());
}


}}}