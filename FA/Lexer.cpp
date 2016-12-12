#include "Lexer.h"

#include "NFABuilderVisitor.h"
#include "AlphabetVisitor.h"
#include "DFABuilder.h"
#include "DFAMinimize.h"
#include "DFAFromNFAConvTraits.h"
#include <LL1RegexParser/RegexParser.h>
#include <SLRRegexParser/RegexParser.h>
#include <RegexSyntaxTreeLib/SyntaxNode.h>
#include <CommonLib/Unicode.h>

namespace mws {

Lexer::Lexer(IStreamExt& is_, const std::vector<StringExt>& regexCol_) 
: 
    _buf(is_), _eof(false)
{
    auto nS = new NFANode();
    
    std::vector<RangeKey> rkVec;
    std::vector<regex::SyntaxNode*> rootVec;

	for (const auto& regex : regexCol_)
    {
		using CharType = StringExt::value_type;

        std::basic_stringstream<CharType> is(regex, std::ios_base::in);
	    mws::td::LL1::RegexParser<common::BufferT<CharType, 4096>> parser(is);

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
        regex::SyntaxNode::Ptr root(rootVec[i]);

        mws::NFABuilderVisitor visitor(rkSet);
        root->accept(visitor);

        std::unique_ptr<NFANode> s(visitor.startState());
        visitor.acceptState()->_regexID = i;

        nS->_transitionMap.insert(s->_transitionMap.begin(), s->_transitionMap.end());
    }

    _dfa = mws::convert(nS);

    mws::minimize(_dfa, rkSet);
}

Token Lexer::next()
{
    mws::Token t;
    
    if (_eof)
    {
        throw common::Exception(_C("Eof"));
    }

    if (!_buf.valid())
    {
        _eof = true;
        t._type = mws::Token::Invalid;
        return t;
    }

    // restart dfa from beginning for each token
    auto* d = _dfa;
   
	auto lexeme = t._lexeme;
	Char buf[8] = { _C('\0') };
	auto pos = _buf.pos();
	auto beg = pos;
	std::size_t regexID = -1;

    for (auto cp = _buf.next(); cp != CP('\0'); cp = _buf.next())
    {
        auto itr = d->_transitionMap.find(cp);
        if (itr == d->_transitionMap.end())
        {
			const auto diff = _buf.pos() - pos; 
            _buf.retract(diff);
            break;
        }

		const auto size = common::utf::Encoder<Char>::encode(cp, buf);
        lexeme.insert(lexeme.end(), buf, buf + size);
        d = itr->second;

		//cache pos of last valid end state
		if (d->_regexID != -1)
		{
			pos = _buf.pos();
			regexID = d->_regexID;
		}
    }

    if (regexID != -1)
    {
		assert(pos > beg);
		t._type = regexID;
		t._lexeme.append(lexeme, 0, pos - beg);
    }
	else
	{
		t._type = mws::Token::Invalid;
	}

    return t;
}

}
