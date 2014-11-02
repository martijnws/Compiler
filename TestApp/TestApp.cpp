// TestApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ToStringVisitor.h"
#include <RegexLL1ParserLib/TableDrivenParser.h>
#include <RegexLL1ParserLib/Parser.h>
#include <SyntaxTreeLib/SyntaxNode.h>
#include <sstream>

int _tmain(int argc, _TCHAR* argv[])
{
	mws::td::LL1::init();

	std::stringstream is("abc(a|ab)*ab*", std::ios_base::in);
	mws::td::LL1::Parser parser(is);

	parser.parse();

	mws::ToStringVisitor visitor;
	parser._astBuilder.root()->accept(visitor);

	std::cout << visitor._result << std::endl;
	return 0;
}

