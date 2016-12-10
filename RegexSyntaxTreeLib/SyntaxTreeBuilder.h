#pragma once

#include "SyntaxNodeFwd.h"
#include <RegexGrammar/ParserHandler.h>
#include <stack>
#include <cassert>

namespace mws { namespace regex { 

class SyntaxTreeBuilder
	:
	public ParserHandler
{
public:
	using Stack = std::stack<SyntaxNode*>;

	void onEof() override;
	void onChoice() override;
	void onConcat() override;
	void onZeroOrOne() override;
	void onZeroToMany() override;
    void onOneToMany() override;
	void onSymbol(const regex::REToken& t_) override;
	void onCharClass() override;
	void onNegate() override;
    void onRngConcat() override;
	void onRng() override;
    void onCharClassSymbol(const regex::CCToken& t_) override;

	SyntaxNode* detach();

private:
	Stack _stack;
};

}}
