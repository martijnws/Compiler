#pragma once

#include <RegexLL1ParserLib/Token.h>
#include <RegexLL1ParserLib/ParserHandler.h>
#include <stack>

namespace mws { namespace ast {

class SyntaxNode;

class SyntaxTreeBuilder
	:
	public td::LL1::ParserHandler
{
public:
	virtual void onEof();
	virtual void onChoice();
	virtual void onConcat();
	virtual void onZeroToMany();
	virtual void onSymbol(const td::LL1::Token& t_);
	virtual void onCharClass();
	virtual void onNegate();
	virtual void onRange();

private:
	std::stack<SyntaxNode*> _stack;
};

}}
