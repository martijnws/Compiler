// TestApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <FA/Lexer.h>
#include <FA/ToStringVisitor.h>
#include <FA/NFABuilderVisitor.h>
#include <FA/DFAInfoBuilderVisitor.h>
#include <FA/AlphabetVisitor.h>
#include <FA/DFABuilder.h>
#include <FA/DFAMinimize.h>
#include <FA/DFAFromFirstFollowPosConvTraits.h>
#include <FA/DFAFromNFAConvTraits.h>
#include <LL1RegexParser/RegexParser.h>
#include <SyntaxTreeLib/SyntaxNode.h>
#include <CommonLib/String.h>

const mws::CharExt* g_regexCol[] =
{
    _CExt("[ \t\n]+"),
    _CExt("[0-9]+"),
    _CExt("if"),
    _CExt("else"),
    _CExt("break"),
    _CExt("continue"),
    _CExt("class"),
    // Note: let a = a-zA-Z0-9, b = _
    // the pattern (a|b)*a(a|b)* reduces to b*a(a|b)*
    //_CExt("[a-zA-Z_]_*[a-zA-Z0-9][a-zA-Z0-9_]*"),
    _CExt("[a-zA-Z_]_*[a-zA-Z][a-zA-Z_]*"),
    _CExt("{"),
    _CExt("}"),
    _CExt(";")
};

//const mws::CharExt* g_regexCol[] =
//{
//	_CExt("a"),
//	_CExt("b"),
//	_CExt("abc"),
//};

void testMatchAndSimulate(mws::DFANode* dfa_, const mws::DFAInfo* s_, const mws::DFAInfo* a_, const mws::Char* str_, bool expect_ = true)
{
	auto res = false;
	res = match(dfa_, a_, str_);
	assert(res == expect_);
	res = simulate(s_, a_, str_);
	assert(res == expect_);
}


int _tmain(int argc, _TCHAR* argv[])
{
	/*
	const auto* regex = "[a-z]";
	std::stringstream is(regex, std::ios::in);
	mws::td::LL1::Parser parser(is);
	parser.parse();
	auto root = parser._astBuilder.detach();
	*/
	

    {
		std::vector<mws::StringExt> regexCol;
		std::copy(g_regexCol, g_regexCol + sizeof(g_regexCol)/sizeof(mws::CharExt*), std::back_inserter(regexCol));

        //const auto* text = _CExt("hello world; if bla ___0_   { continue; } else elsbla else1234 { bla 1234 break; }");
        const auto* text = _CExt("hello world; if bla ___A_   { continue; } else elsbla else1234 { bla 1234 break; }");


        //const auto* text = _CExt("ababc");
        mws::StringStreamExt is(text, std::ios_base::in);

        mws::Lexer lexer(is, regexCol);
        for (auto t = lexer.next(); t._type != mws::Token::Invalid; t = lexer.next())
        {
            stdOut << _C("lexeme = ") << t._lexeme << _C(", type = ") << g_regexCol[t._type] << std::endl;
        }
    }

    const auto* regex = _CExt("abc[^a-zA-Z]*def|abc(a|ab)*ab*");
	mws::StringStreamExt is(regex, std::ios_base::in);
	mws::td::LL1::RegexParser<mws::common::BufferExt> parser(is);

	parser.parse();
    mws::regex::SyntaxNode::Ptr root(parser._astBuilder.detach());

    {
        mws::ToStringVisitor visitor;
	    root->accept(visitor);
	    stdOut << visitor._result << std::endl;
    }

    {
        mws::AlphabetVisitor alphabetVisitor;
        root->accept(alphabetVisitor);

        const auto rkSet = mws::getDisjointRangeSet(alphabetVisitor._rkVec);

        //mws::NFABuilderVisitor visitor(rkSet);
        //using DFAItem = mws::NFANode;
        
        mws::DFAInfoBuilderVisitor visitor(rkSet);
        using DFAItem = mws::DFAInfo;
        
        //using DFANode = mws::DFANode<DFAItem>;

        root->accept(visitor);

        auto s = visitor.startState();
        auto a = visitor.acceptState();

        mws::DFANode* dfa = mws::convert(s);
        mws::minimize(dfa, rkSet);

        testMatchAndSimulate(dfa, s, a, _C("abc@##^def"));
        testMatchAndSimulate(dfa, s, a, _C("abcaabaaabaaababaaabbb"));

        testMatchAndSimulate(dfa, s, a, _C("abc@##^ddef"), false);
        testMatchAndSimulate(dfa, s, a, _C("abcaabaaabaaabbabaaabbb"), false);
    }

	return 0;
}

