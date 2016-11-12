#pragma once

#include "ParserState.h"
#include "ParserTable.h"
#include "IParser.h"
#include <Grammar/Grammar.h>

namespace mws { namespace td { namespace LL1 {

template<typename LexerT>
class ParserDriver
:
    public IParser
{
public:
	ParserDriver(LexerT& lexer_, grammar::Handler& h_, const grammar::Grammar& grammar_, const ParserTable& parserTable_, const SubParserMap& subParserCol_)
    :
		_lexer(lexer_),
        _h(h_), 
        _grammar(grammar_), 
        _parserTable(parserTable_), 
        _subParserCol(subParserCol_)
	{
		
	}

    void parse(grammar::Token& cur_) override;

private:
	LexerT&                 _lexer;
    grammar::Handler&       _h;
    const grammar::Grammar& _grammar;
    const ParserTable&      _parserTable;
    const SubParserMap&     _subParserCol;
};

template<typename LexerT>
void ParserDriver<LexerT>::parse(grammar::Token& cur_)
{
    ParserState<LexerT> st(_lexer, cur_);
    st.init();

	using GSEntry = std::pair<grammar::GrammarSymbol, bool>;

	grammar::TokenStore store;

    // type = 0 indicates this GrammarSymbol is an instance of NT at grammar[0]. That is, it is an instance of Start
    grammar::GrammarSymbol startSymbol = grammar::n(0);
	std::vector<GSEntry> stack;
	stack.push_back(std::make_pair(startSymbol, false));

	while (!stack.empty())
	{
        // copy by value since later on st.cur() will change
        const auto tokCur = st.cur();

        auto& entry = stack.back();
	    const auto& gs = entry.first;

		if (gs._isTerminal)
		{
            //std::cout << "match Terminal: " << tokCur._lexeme << std::endl;

            auto itr = _subParserCol.find(gs._type);
            const bool isSGSS = itr != _subParserCol.end();
			// match
			st.m(gs._type, !isSGSS);
           
            gs._action(_h, tokCur, store);
			stack.pop_back();
          
            if (isSGSS)
            {
                const auto& parser = itr->second;
                parser->parse(st.cur());
            }

			continue;
		}

		// is entry expanded?
		if (entry.second)
		{
            //std::cout << "Pop NonTerminal " << _grammar[gs._type]._name << std::endl;
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