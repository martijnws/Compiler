// TestApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ToStringVisitor.h"
#include "NFABuilderVisitor.h"
#include "NFABuilderVisitorV2.h"
#include "DFAInfoBuilderVisitor.h"
#include "DFAInfo.h"
#include "DFABuilder.h"
#include "DFADirectBuilder.h"
#include <RegexLL1ParserLib/TableDrivenParser.h>
#include <RegexLL1ParserLib/Parser.h>
#include <SyntaxTreeLib/SyntaxNode.h>
#include <sstream>

int _tmain(int argc, _TCHAR* argv[])
{
	mws::td::LL1::init();

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
        mws::DFAInfoBuilderVisitor visitor;
        root->accept(visitor);
    }

    {

        /*using namespace mws;
        
        mws::NFABuilderVisitorV2 visitor;
	    root->accept(visitor);

        auto s = visitor.startState();
        auto a = visitor.acceptState();

        DFANode* dfa = mws::convert(s);*/
        

        using namespace mws::direct;

        mws::DFAInfoBuilderVisitor visitor;
        root->accept(visitor);

        auto s = visitor.startState();
        auto a = visitor.acceptState();

        DFANode* dfa = mws::direct::convert(s);

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

