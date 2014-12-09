#include "Lexer.h"

#include "ToStringVisitor.h"
#include "NFABuilderVisitor.h"
#include "DFABuilder.h"
#include "DFAFromNFAConvTraits.h"
#include <RegexLL1ParserLib/Parser.h>
#include <SyntaxTreeLib/SyntaxNode.h>
#include <sstream>

namespace mws {

const char* g_regexCol[] =
{
    "  *",
    "[0-9][0-9]*",
    "if",
    "else",
    "break",
    "continue",
    "class",
    "[a-zA-Z_][a-zA-Z0-9_]*",
    "{",
    "}",
    ";"
};

Lexer::Lexer(std::istream& is_) 
: 
    _buf(is_), _eof(false)
{
    auto nS = new NFANode();
    
    for (std::size_t i = 0; i < sizeof(g_regexCol)/sizeof(char*); ++i)
    {
        auto regex = g_regexCol[i];

        std::stringstream is(regex, std::ios_base::in);
	    mws::td::LL1::Parser parser(is);

	    parser.parse();
        mws::ast::SyntaxNodePtr root(parser._astBuilder.detach());
  
        mws::NFABuilderVisitor visitor;

        root->accept(visitor);

        std::unique_ptr<NFANode> s(visitor.startState());
        visitor.acceptState()->_accept = true;
        visitor.acceptState()->_regex = i;

        nS->_transitionMap.insert(s->_transitionMap.begin(), s->_transitionMap.end());
    }

    mws::DFATraits<NFANode>::preprocess(nS);
    _dfa = mws::convert(nS);
}
   
const char* Lexer::regex(std::size_t type) const
{
    assert(type < sizeof(g_regexCol)/sizeof(char*));
    return g_regexCol[type];
}

Token Lexer::next()
{
    mws::Token t;
    
    if (_eof)
    {
        throw common::Exception("Eof");
    }

    if (!_eof && !_buf.valid())
    {
        _eof = true;
        t._type = (std::size_t)-1;
        return t;
    }

    // restart dfa from beginning for each token
    DFANode* dfa = _dfa;
   
    for (auto c = _buf.next(); c != '\0'; c = _buf.next())
    {
        auto itr = dfa->_transitionMap.find(c);
        if (itr == dfa->_transitionMap.end())
        {
            _buf.retract(1);
            break;
        }

        t._lexeme += c;
        dfa = itr->second;
    }

    std::size_t regex = -1;
    for (auto n : dfa->_items)
    {
        if (n->_accept)
        {
            regex = std::min(regex, n->_regex);
            //std::cout << "accept = " << this->regex(n->_regex) << std::endl;
        }
    }

    if (regex == (std::size_t)-1)
    {
        throw common::Exception("Invalid Token");
    }

    t._type = regex;
    return t;
}

}
