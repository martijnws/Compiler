#pragma once

#include <RegexGrammar/ParserHandler.h>
#include <stack>
#include <cassert>

namespace mws { namespace ast {

class SyntaxNode;

class SyntaxTreeBuilder
	:
	public regex::ParserHandler
{
public:
	void onEof() override;
	void onChoice() override;
	void onConcat() override;
	void onZeroOrOne() override;
	void onZeroToMany() override;
    void onOneToMany() override;
	void onSymbol(const grammar::Token& t_) override;
	void onCharClass() override;
	void onNegate() override;
    void onRngConcat() override;
	void onRng() override;
    void onCharClassSymbol(const grammar::Token& t_) override;

	SyntaxNode* detach()
	{
		if (_stack.size())
		{
			assert(_stack.size() == 1);
			SyntaxNode* n = _stack.top();
            _stack.pop();
            return n;
		}
		else
		{
			return nullptr;
		}
	}

private:
	std::stack<SyntaxNode*> _stack;
};

}}
