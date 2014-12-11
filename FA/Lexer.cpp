#include "Lexer.h"

#include "ToStringVisitor.h"
#include "NFABuilderVisitor.h"
#include "AlphabetVisitor.h"
#include "DFABuilder.h"
#include "DFAMinimize.h"
#include "DFAFromNFAConvTraits.h"
#include <RegexLL1ParserLib/Parser.h>
#include <SyntaxTreeLib/SyntaxNode.h>
#include <sstream>

namespace mws {

const char* g_regexCol[] =
{
    "[ \t\n]+",
    "[0-9]+",
    "if",
    "else",
    "break",
    "continue",
    "class",
    // Note: let a = a-zA-Z0-9, b = _
    // the pattern (a|b)*a(a|b)* reduces to b*a(a|b)*
    "[a-zA-Z_]_*[a-zA-Z0-9][a-zA-Z0-9_]*",
    "{",
    "}",
    ";"
};

Lexer::Lexer(std::istream& is_) 
: 
    _buf(is_), _eof(false)
{
    auto nS = new NFANode();
    
    std::vector<RangeKey> rkVec;
    std::vector<mws::ast::SyntaxNode*> rootVec;

    for (std::size_t i = 0; i < sizeof(g_regexCol)/sizeof(char*); ++i)
    {
        auto regex = g_regexCol[i];

        std::stringstream is(regex, std::ios_base::in);
	    mws::td::LL1::Parser parser(is);

	    parser.parse();
        mws::ast::SyntaxNode* root(parser._astBuilder.detach());
  
        mws::AlphabetVisitor alphabetVisitor;
        root->accept(alphabetVisitor);
        rkVec.insert(rkVec.end(), alphabetVisitor._rkVec.begin(), alphabetVisitor._rkVec.end());

        rootVec.push_back(root);
    }

    std::set<RangeKey, RangeKey::Less> rkSet = mws::getDisjointRangeSet(rkVec);

    for (std::size_t i = 0; i < rootVec.size(); ++i)
    {
        mws::ast::SyntaxNodePtr root(rootVec[i]);

        mws::NFABuilderVisitor visitor(rkSet);
        root->accept(visitor);

        std::unique_ptr<NFANode> s(visitor.startState());
        visitor.acceptState()->_accept = true;
        visitor.acceptState()->_regex = i;

        nS->_transitionMap.insert(s->_transitionMap.begin(), s->_transitionMap.end());
    }

    _dfa = mws::convert(nS);

    mws::minimize(_dfa, rkSet);
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
