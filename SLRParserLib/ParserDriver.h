#pragma once

#include "ParserTable.h"
#include <ParserLib/ParserState.h>
#include <ParserLib/IParser.h>
#include <Grammar/Grammar.h>

namespace mws { namespace td { namespace SLR {

template<typename LexerT>
class ParserDriver
:
    public IParser
{
public:
	using TokenEnum    = typename LexerT::Token::Enum;
	using SubParserMap = std::map<TokenEnum, IParser*>;

	ParserDriver(LexerT& lexer_, grammar::Handler& h_, const grammar::Grammar& grammar_, const ParserTable& parserTable_, const SubParserMap& subParserCol_)
    :
		_lexer(lexer_),
        _h(h_), 
        _grammar(grammar_), 
        _parserTable(parserTable_), 
        _subParserCol(subParserCol_)
	{
		
	}

    void parse() override;

private:
	LexerT&                 _lexer;
    grammar::Handler&       _h;
    const grammar::Grammar& _grammar;
    const ParserTable&   _parserTable;
    const SubParserMap&     _subParserCol;
};

template<typename LexerT>
void ParserDriver<LexerT>::parse()
{
    ParserState<LexerT> st(_lexer);
    st.init();

    grammar::TokenStore store;

    std::stack<uint8_t> stack;
    //std::cout << "stack.psh: " << (int)_parserTable.start()->_label << std::endl;
    stack.push(_parserTable.start()->_label);

    // remember last terminal
    grammar::Token lastTerminal;

    for (bool done = false; !done; )
    {
        GrammarSymbol t = { st.cur()._type, true };

        const auto entry = _parserTable.action(stack.top(), t._type);
        switch(entry._action)
        {
            case ParserTable::Action::Shift:
            {
                lastTerminal = st.cur();
                //std::cout << "Shift: lexeme=" << lastTerminal._lexeme << std::endl; 
                // Shift
                //std::cout << "stack.psh: " << (int)entry._state << std::endl;
                stack.push(entry._state);

                auto itr = _subParserCol.find(lastTerminal._type);
                if (itr != _subParserCol.end())
                {
                    const auto& parser = itr->second;
                    parser->parse(st.cur());
                }

                st.next();
                break;
            }
            case ParserTable::Action::Reduce:
            {
                const auto& nt = _grammar[entry._prod._head];
                const auto& prod = nt._prodList[entry._prod._body];

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

                //uint8_t state = _parserTable.goTo(stack.top(), entry._prod._head);
                //std::cout << "stack.psh: " << (int)state << std::endl;
                stack.push(_parserTable.goTo(stack.top(), entry._prod._head));

                break;
            }
            case ParserTable::Action::Accept:
            {
                //std::cout << "Accept:" << std::endl;;
                //Token::Eof does not work because subGrammars may have different 'Eof' tokens
                //assert(t._type == grammar::Token::Eof);
                done = true;
                break;
            }
            case ParserTable::Action::Error:
            {
                throw common::Exception(_C("SLRParser error: No valid action from state"));
            }
            default:
            {
                throw common::Exception(_C("SLRParser error: Unknown action"));
            }
        }
    }
	
	assert(st.eof() || st.invalid());
}

}}}