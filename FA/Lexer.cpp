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
#include <algorithm>

namespace mws {

Lexer::Lexer(IStreamExt& is_, const std::vector<IPair>& regexCol_) 
: 
    _buf(is_), _eof(false)
{
    auto nS = new NFANode();
    
    std::vector<RangeKey> rkVec;
    std::vector<regex::SyntaxNode*> rootVec;
	rootVec.reserve(regexCol_.size());

	for (const auto& pair : regexCol_)
    {
		using CharType = StringExt::value_type;

        std::basic_stringstream<CharType> is(pair.regex, std::ios_base::in);
	    mws::td::LL1::RegexParser<common::BufferT<CharType, 4096>> parser(is);

	    parser.parse();
        auto* root = parser._astBuilder.detach();
		assert(root != nullptr);
        rootVec.push_back(root);
  
        mws::AlphabetVisitor alphabetVisitor;
        root->accept(alphabetVisitor);
        rkVec.insert(rkVec.end(), alphabetVisitor._rkVec.begin(), alphabetVisitor._rkVec.end());
    }

    std::set<RangeKey> rkSet = mws::getDisjointRangeSet(rkVec);

    for (auto i = 0ul; i < rootVec.size(); ++i)
    {
        regex::SyntaxNode::Ptr root(rootVec[i]);
		assert(root);

        mws::NFABuilderVisitor visitor(rkSet);
        root->accept(visitor);

        std::unique_ptr<NFANode> s(visitor.startState());
        visitor.acceptState()->_regexID = regexCol_[i].type;

        nS->_transitionMap.insert(s->_transitionMap.begin(), s->_transitionMap.end());
    }

    _dfa = mws::convert(nS);

    mws::minimize(_dfa, rkSet);
}

grammar::Token::Type Lexer::next(String& lexeme_)
{
    if (_eof)
    {
        throw common::Exception(_C("Eof"));
    }

	grammar::Token::Type type = grammar::Token::Invalid;

    if (!_buf.valid())
    {
        _eof = true;
        return type;
    }

    // restart dfa from beginning for each token
    auto* d = _dfa;
   
	String lexeme;
	Char buf[8]; // = { _C('\0') };
	auto pos = _buf.pos();
	auto beg = pos;
	std::size_t regexID = -1;

    for (auto cp = _buf.next(); cp != CP('\0'); cp = _buf.next())
    {
        auto itr = d->_transitionMap.find(cp);
        if (itr == d->_transitionMap.end())
        {
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

	//retract to position where valid regexID was found (or beg, in case no match was found at all)
	_buf.retract(_buf.pos() - pos);

	assert(regexID != -1 ? pos > beg : pos == beg);
    if (regexID != -1)
    {
		type = regexID;
		lexeme_.append(lexeme, 0, pos - beg);
    }

	return type;
}

}
