#include "Lexer.h"

#include "NFABuilderVisitor.h"
#include "AlphabetVisitor.h"
#include "DFABuilder.h"
#include "DFAMinimize.h"
#include "DFAFromNFAConvTraits.h"
#include <LL1RegexParser/RegexParser.h>
#include <SLRRegexParser/RegexParser.h>
#include <SyntaxTreeLib/SyntaxNode.h>
#include <CommonLib/Unicode.h>

namespace mws {

const Char* g_regexCol[] =
{
    _C("[ \t\n]+"),
    _C("[0-9]+"),
    _C("if"),
    _C("else"),
    _C("break"),
    _C("continue"),
    _C("class"),
    // Note: let a = a-zA-Z0-9, b = _
    // the pattern (a|b)*a(a|b)* reduces to b*a(a|b)*
    _C("[a-zA-Z_]_*[a-zA-Z0-9][a-zA-Z0-9_]*"),
    _C("{"),
    _C("}"),
    _C(";")
};

Lexer::Lexer(IStreamExt& is_) 
: 
    _buf(is_), _eof(false)
{
	//does not read from external source but from internal string literals
	using RegexBuf = common::BufferT<Char, 4096>;

    auto nS = new NFANode();
    
    std::vector<RangeKey> rkVec;
    std::vector<mws::ast::SyntaxNode*> rootVec;

    for (auto i = 0ul; i < sizeof(g_regexCol)/sizeof(Char*); ++i)
    {
        auto regex = g_regexCol[i];

        StringStream is(regex, std::ios_base::in);
	    mws::td::LL1::RegexParser<RegexBuf> parser(is);

	    parser.parse();
        auto* root = parser._astBuilder.detach();
  
        mws::AlphabetVisitor alphabetVisitor;
        root->accept(alphabetVisitor);
        rkVec.insert(rkVec.end(), alphabetVisitor._rkVec.begin(), alphabetVisitor._rkVec.end());

        rootVec.push_back(root);
    }

    std::set<RangeKey> rkSet = mws::getDisjointRangeSet(rkVec);

    for (auto i = 0ul; i < rootVec.size(); ++i)
    {
        mws::ast::SyntaxNodePtr root(rootVec[i]);

        mws::NFABuilderVisitor visitor(rkSet);
        root->accept(visitor);

        std::unique_ptr<NFANode> s(visitor.startState());
        visitor.acceptState()->_regexID = i;

        nS->_transitionMap.insert(s->_transitionMap.begin(), s->_transitionMap.end());
    }

    _dfa = mws::convert(nS);

    mws::minimize(_dfa, rkSet);
}
   
const Char* Lexer::regex(std::size_t type) const
{
    assert(type < sizeof(g_regexCol)/sizeof(Char*));
    return g_regexCol[type];
}

Token Lexer::next()
{
    mws::Token t;
    
    if (_eof)
    {
        throw common::Exception(_C("Eof"));
    }

    if (!_eof && !_buf.valid())
    {
        _eof = true;
        t._type = mws::Token::Invalid;
        return t;
    }

    // restart dfa from beginning for each token
    auto* d = _dfa;
   
	Char buf[8] = { _C('\0') };
    for (auto cp = _buf.next(); cp != CP('\0'); cp = _buf.next())
    {
        auto itr = d->_transitionMap.find(cp);
        if (itr == d->_transitionMap.end())
        {
            _buf.retract(1);
            break;
        }

		const auto size = common::utf::Encoder<Char>::encode(cp, buf);
        t._lexeme.insert(t._lexeme.end(), buf, buf + size);
        d = itr->second;
    }

    if (d->_regexID == -1)
    {
        throw common::Exception(_C("Invalid Token"));
    }

    t._type = d->_regexID;
    return t;
}

}
