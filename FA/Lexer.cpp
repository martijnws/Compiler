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

Lexer::Lexer(IStreamExt& is_, const TokenInfoCol& tokenInfoCol_) 
: 
    _buf(is_), _eof(false)
{
	if (tokenInfoCol_.empty())
	{
		throw common::Exception(_S("TokenInfo cannot be empty"));
	}

    auto nS = new NFANode();
    
	const auto itrMax = std::max_element(tokenInfoCol_.begin(), tokenInfoCol_.end(), 
		[](const auto& ti1_, const auto& ti2_) { return ti1_.type < ti2_.type; });
	assert(itrMax != tokenInfoCol_.end());

	_tokenInfoCol.resize(itrMax->type + 1);

    std::vector<RangeKey> rkVec;
    std::vector<regex::SyntaxNode*> rootVec;
	rootVec.reserve(tokenInfoCol_.size());

	for (const auto& ti : tokenInfoCol_)
    {
		using CharType = StringExt::value_type;
		
		//cache for constant time lookup on type
		_tokenInfoCol[ti.type] = ti;

        std::basic_stringstream<CharType> is(ti.regex, std::ios_base::in);
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
        visitor.acceptState()->_regexID = tokenInfoCol_[i].type;

        nS->_transitionMap.insert(s->_transitionMap.begin(), s->_transitionMap.end());
    }

    _dfa = mws::convert(nS);

    mws::minimize(_dfa, rkSet);
}

bool Lexer::next(String& lexeme_, TokenID& type_)
{
	auto result = false;

	for (auto skip = true; skip; result = next(lexeme_, type_, skip));

	return result;
}

bool Lexer::next(String& lexeme_, TokenID& type_, bool& skip_)
{
	skip_ = false;

    if (_eof)
    {
        throw common::Exception(_S("Eof"));
    }

    if (!_buf.valid())
    {
        _eof = true;
        return false;
    }

    // restart dfa from beginning for each token
    auto* d = _dfa;
   
	String lexeme;
	Char buf[8]; // = { _C('\0') };
	auto pos = _buf.pos();
	auto beg = pos;
	auto regexID = InvalidTokenID;

    //for (auto cp = _buf.next(); _buf.valid(); cp = _buf.next())
	do
    {
		auto cp = _buf.next();

        auto itr = d->_transitionMap.find(cp);
        if (itr == d->_transitionMap.end())
        {
            break;
        }

		const auto size = common::utf::Encoder<Char>::encode(cp, buf);
        lexeme.insert(lexeme.end(), buf, buf + size);
        d = itr->second;

		//cache pos of last valid end state
		if (d->_regexID != InvalidTokenID)
		{
			pos = _buf.pos();
			regexID = d->_regexID;
		}
    }
	while (_buf.valid());

	//retract to position where valid regexID was found (or beg, in case no match was found at all)
	_buf.retract(_buf.pos() - pos);

	const auto result = regexID != InvalidTokenID;
	assert(result ? pos > beg : pos == beg);

    if (result)
    {
		type_ = regexID;
		lexeme_.append(lexeme, 0, pos - beg);
		skip_ = _tokenInfoCol[type_].skip;
    }

	return result;
}

}
