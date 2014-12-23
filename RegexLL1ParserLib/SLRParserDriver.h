#pragma once

#include "ParserState.h"
#include "SLRParserTable.h"
#include "IParser.h"
#include <Grammar/Grammar.h>

namespace mws { namespace td { namespace LL1 {

template< template<typename> class LexerT>
class SLRParserDriver
:
    public IParser
{
public:
	SLRParserDriver(grammar::Handler& h_, const grammar::Grammar& grammar_, const SLRParserTable& parserTable_, const SubParserMap& subParserCol_)
    :
        _h(h_), 
        _grammar(grammar_), 
        _parserTable(parserTable_), 
        _subParserCol(subParserCol_)
	{
		
	}

    void parse(common::Buffer& buf_, grammar::Token& cur_) override;

private:
    grammar::Handler&       _h;
    const grammar::Grammar& _grammar;
    const SLRParserTable&   _parserTable;
    const SubParserMap&     _subParserCol;
};

template< template<typename> class LexerT>
void SLRParserDriver<LexerT>::parse(common::Buffer& buf_, grammar::Token& cur_)
{
    ParserState<common::Buffer, LexerT<common::Buffer>> st(buf_, cur_);
    st.init();

    grammar::TokenStore store;

    std::stack<const LR0State*> stack;

    auto state = _parserTable.start();
    stack.push(state);

    // remember last terminal
    grammar::Token lastTerminal;

    for (bool done = false; !done; )
    {
        GrammarSymbol t = { st.cur()._type, true };

        auto itr = state->_transitionMap.find(t);
        // Shift possible?
        if (itr != state->_transitionMap.end())
        {
            lastTerminal = st.cur();
            //std::cout << "Shift: lexeme=" << lastTerminal._lexeme << std::endl; 
            // Shift
            state = itr->second;
            stack.push(state);

            auto itr = _subParserCol.find(lastTerminal._type);
            if (itr != _subParserCol.end())
            {
                const auto& parser = itr->second;
                parser->parse(st.buf(), st.cur());
            }
            else
            {
                st.next();
            }
            
            continue;
        }

        // Reduction possible?
        bool reduce = false;
        for (const auto& item : *state->_itemSet)
        {
            const auto& nt = _grammar[item._nt];
            const auto& prod = nt._prodList[item._prod];

            // Reduce (dot to the right, curTok in head.follow)
            if (prod._gsList.size() == item._dot && nt._follow.find(t._type) != nt._follow.end())
            {
                //std::cout << "Reduce: " << nt._name << " := ";

                // tracing back the path through the LR0 DFA by prod.size steps
                assert(stack.size() >= prod._gsList.size());
                for (std::size_t i = 0; i < prod._gsList.size(); ++i)
                {
                    // actions are executed from left to right
                    const auto& gs = prod._gsList[i];

                    //print(gs, _grammar);
                    //std::cout << " ";

                    // lastTerminal is only going to be accurate if Symbol/ID tokens have a dedicated production
                    gs._action(_h, lastTerminal, store);

                    // states (corresponding to GrammarSymbols of production) are popped right to left
                    stack.pop();
                }
                //std::cout << std::endl;

                done = item._nt == 0;
                if (!done)
                {
                    state = stack.top();
                    GrammarSymbol gs = { item._nt, false };
                    auto itr = state->_transitionMap.find(gs);
                    assert(itr != state->_transitionMap.end());

                    // Shift NonTerminal just reduced
                    state = itr->second;
                    stack.push(state);
                }

                reduce = true;
                break;
            }
        }

        if (!reduce)
        {
            throw common::Exception("SLRParser error: No valid action from state");
        }
    }


	//assert(st.eof());
}


}}}