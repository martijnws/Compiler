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
#include <RegexLL1ParserLib/TableDrivenParser.h>
#include <RegexLL1ParserLib/Parser.h>
#include <SyntaxTreeLib/SyntaxNode.h>
#include <sstream>

int _tmain(int argc, _TCHAR* argv[])
{
	mws::td::LL1::init();

    {
        const char* text = "hello world; if bla ___0_   { continue; } else elsbla else1234 { bla 1234 break; }";
        std::stringstream is(text, std::ios_base::in);

        mws::Lexer lexer(is);
        for (mws::Token t = lexer.next(); t._type != (std::size_t)-1; t = lexer.next())
        {
            std::cout << "lexeme = " << t._lexeme << ", type = " << lexer.regex(t._type) << std::endl;
        }
    }

    const char* regex = "abc[^a-zA-Z]*def|abc(a|ab)*ab*";
	std::stringstream is(regex, std::ios_base::in);
	mws::td::LL1::Parser parser(is);

	parser.parse();
    mws::ast::SyntaxNodePtr root(parser._astBuilder.detach());

    {
        mws::ToStringVisitor visitor;
	    root->accept(visitor);
	    std::cout << visitor._result << std::endl;
    }

    {
        mws::AlphabetVisitor alphabetVisitor;
        root->accept(alphabetVisitor);

        std::set<mws::RangeKey, mws::RangeKey::Less> rkSet = mws::getDisjointRangeSet(alphabetVisitor._rkVec);

        mws::NFABuilderVisitor visitor(rkSet);
        using DFAItem = mws::NFANode;
        
        //mws::DFAInfoBuilderVisitor visitor(rkSet);
        //using DFAItem = mws::DFAInfo;
        
        //using DFANode = mws::DFANode<DFAItem>;

        root->accept(visitor);

        auto s = visitor.startState();
        auto a = visitor.acceptState();

        mws::DFANode* dfa = mws::convert(s);
        dfa->_regexID = a->_regexID;
        mws::minimize(dfa, rkSet);

        const char* str1 = "abc@##^def";
        const char* str2 = "abcaabaaabaaababaaabbb";

        const char* str3 = "abc@##^ddef";
        const char* str4 = "abcaabaaabaaabbabaaabbb";
        bool res = false;
        
        res = match(dfa, a, str1);
        assert(res);
        res = simulate(s, a, str1);
        assert(res);

        res = match(dfa, a, str2);
        assert(res);
        res = simulate(s, a, str2);
        assert(res);

        res = match(dfa, a, str3);
        assert(!res);
        res = simulate(s, a, str3);
        assert(!res);

        res = match(dfa, a, str4);
        assert(!res);
        res = simulate(s, a, str4);
        assert(!res);
    }

	return 0;
}

