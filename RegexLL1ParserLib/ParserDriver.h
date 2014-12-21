#pragma once

#include "ParserState.h"
#include "ParserTable.h"
#include "IParser.h"
#include <Grammar/Grammar.h>

namespace mws { namespace td { namespace LL1 {

template< template<typename> class LexerT>
class ParserDriver
:
    public IParser
{
public:
	ParserDriver(grammar::Handler& h_, const grammar::Grammar& grammar_, const ParserTable& parserTable_, const std::vector<std::pair<IParser*, bool>>& subParserCol_)
    :
        _h(h_), 
        _grammar(grammar_), 
        _parserTable(parserTable_), 
        _subParserCol(subParserCol_)
	{
		
	}

    void parse(common::Buffer& buf_, grammar::Token& cur_);
    void parse(const grammar::GrammarSymbol& startSymbol_, common::Buffer& buf_, grammar::Token& cur_) override;

private:
    grammar::Handler&                             _h;
    const grammar::Grammar&                       _grammar;
    const ParserTable&                            _parserTable;
    const std::vector<std::pair<IParser*, bool>>& _subParserCol;
};

template< template<typename> class LexerT>
void ParserDriver<LexerT>::parse(common::Buffer& buf_, grammar::Token& cur_)
{
    // type = 0 indicates this GrammarSymbol is an instance of NT at grammar[0]. That is, it is an instance of Start
    grammar::GrammarSymbol startSymbol = grammar::n(0);
    parse(startSymbol, buf_, cur_);
}

template< template<typename> class LexerT>
void ParserDriver<LexerT>::parse(const grammar::GrammarSymbol& startSymbol_, common::Buffer& buf_, grammar::Token& cur_)
{
    ParserState<common::Buffer, LexerT<common::Buffer>> st(buf_, cur_);
    st.init();

	using GSEntry = std::pair<grammar::GrammarSymbol, bool>;

	grammar::TokenStore store;

	std::vector<GSEntry> stack;
	stack.push_back(std::make_pair(startSymbol_, false));

	while (!stack.empty())
	{
		auto& entry = stack.back();
		const auto& gs = entry.first;

        // copy by value since later on st.cur() will change
        const auto tokCur = st.cur();

        if (gs.isSubGrammarStartSymbol())
        {
            assert(gs._subGrammarParserID < _subParserCol.size());
            const auto& parserInfo = _subParserCol[gs._subGrammarParserID];
            // Different parser same Grammar?
            if (parserInfo.second)
            {
                // in sub parse, this startSymbol becomes the toplevel startsymbol.
                grammar::GrammarSymbol startSymbol(gs);
                // To prevent infinite recursion.
                startSymbol._subGrammarParserID = grammar::GrammarSymbol::InvalidParserID;
                assert(!startSymbol.isSubGrammarStartSymbol());

                parserInfo.first->parse(startSymbol, st.buf(), st.cur());
            }
            // Different Parser, different Grammar
            else
            {
                grammar::GrammarSymbol startSymbol = grammar::n(0);

                parserInfo.first->parse(startSymbol, st.buf(), st.cur());
            }

            // Do not execute action. It is already executed in sub parse
			stack.pop_back();
			continue;
        }

		if (gs._isTerminal)
		{
			// match
			st.m(static_cast<grammar::Token::Type>(gs._type), gs._fetchNext);
            // mark entry as expanded
		    entry.second = true;

		}

		// is entry expanded?
		if (entry.second)
		{
			gs._action(_h, tokCur, store);
			stack.pop_back();
			continue;
		}

		assert(!gs._isTerminal);
		// mark entry as expanded
		entry.second = true;

		// expand
		const auto& prod = _parserTable.expand(_grammar, gs._type, tokCur._type);
        // signed i used to detect >= condition
		for (int i = static_cast<int>(prod._gsList.size()) - 1; i >= 0; --i)
	    {
			stack.push_back(std::make_pair(prod._gsList[i], false));
		}
	}

	//assert(st.eof());
}


}}}