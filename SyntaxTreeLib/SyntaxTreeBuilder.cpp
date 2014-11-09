#include "SyntaxTreeBuilder.h"
#include "SyntaxNode.h"
#include <cassert>

namespace mws { namespace ast {

using Stack = std::stack<SyntaxNode*>;

void push(Stack& stack_, SyntaxNode* n_)
{
	stack_.push(n_);
}

SyntaxNode* pop(Stack& stack_)
{
	assert(!stack_.empty());

	SyntaxNode* n = stack_.top();
	stack_.pop();
	return n;
}

template<typename OperatorT>
void onUnaryOp(Stack& stack_)
{
	SyntaxNode* n = pop(stack_);
	push(stack_, new AcceptorImpl<OperatorT>(n));
}

template<typename OperatorT>
void onBinaryOp(Stack& stack_)
{
	SyntaxNode* rhs = pop(stack_);
	SyntaxNode* lhs = pop(stack_);
	push(stack_, new AcceptorImpl<OperatorT>(lhs, rhs));
}

void SyntaxTreeBuilder::onEof()
{
	//can be zero if empty expression is allowed
	assert(_stack.size() <= 1);
}

void SyntaxTreeBuilder::onChoice()
{
	onBinaryOp<Choice>(_stack);
}

void SyntaxTreeBuilder::onConcat()
{
	onBinaryOp<Concat>(_stack);
}

void SyntaxTreeBuilder::onZeroToMany()
{
	onUnaryOp<ZeroToMany>(_stack);
}

void SyntaxTreeBuilder::onSymbol(const td::LL1::Token& t_)
{
	push(_stack, new AcceptorImpl<Symbol>(t_._lexeme));
}

void SyntaxTreeBuilder::onCharClass()
{
	onUnaryOp<CharClass>(_stack);
}

void SyntaxTreeBuilder::onNegate()
{
	onUnaryOp<Negate>(_stack);
}

void SyntaxTreeBuilder::onRngConcat()
{
	onBinaryOp<RngConcat>(_stack);
}

void SyntaxTreeBuilder::onRng()
{
	onBinaryOp<Rng>(_stack);
}

void SyntaxTreeBuilder::onCharClassSymbol(const td::LL1::Token& t_)
{
	push(_stack, new AcceptorImpl<CharClassSymbol>(t_._lexeme));
}

}}
