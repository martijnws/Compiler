#pragma once

#include <RegexLL1ParserLib/Token.h>
#include <RegexLL1ParserLib/ParserHandler.h>
#include <stack>
#include <cassert>

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
    virtual void onRngConcat();
	virtual void onRng();
    virtual void onCharClassSymbol(const td::LL1::Token& t_);

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
