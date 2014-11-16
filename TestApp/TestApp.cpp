// TestApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ToStringVisitor.h"
#include "NFABuilderVisitor.h"
#include "NFABuilderVisitorV2.h"
#include "DFABuilder.h"
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
        mws::NFABuilderVisitorV2 visitor;
	    root->accept(visitor);

        mws::DFANode* dfa = mws::convert(visitor._result._s);

        const char* str1 = "abc@##^def";
        const char* str2 = "abcaabaaabaaababaaabbb";

        const char* str3 = "abc@##^ddef";
        const char* str4 = "abcaabaaabaaabbabaaabbb";
        bool res = false;
        
        res = match(dfa, visitor._result._f, str1);
        assert(res);
        res = simulate(visitor._result, str1);
        assert(res);

        res = match(dfa, visitor._result._f, str2);
        assert(res);
        res = simulate(visitor._result, str2);
        assert(res);

        res = match(dfa, visitor._result._f, str3);
        assert(!res);
        res = simulate(visitor._result, str3);
        assert(!res);

        res = match(dfa, visitor._result._f, str4);
        assert(!res);
        res = simulate(visitor._result, str4);
        assert(!res);
    }

	return 0;
}

