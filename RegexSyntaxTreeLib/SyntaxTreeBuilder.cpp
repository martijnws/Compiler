#include "SyntaxTreeBuilder.h"
#include "SyntaxNode.h"
#include "Visitor.h"
#include <RegexLexer/Token.h>
#include <SyntaxTreeLib/Util.h>
#include <cassert>

namespace mws { namespace regex {

using Stack = std::stack<SyntaxNode*>;
using Util = ast::Util<SyntaxNode, Stack>;

SyntaxNode* SyntaxTreeBuilder::detach()
{
	return Util::popLast(_stack);
}

void SyntaxTreeBuilder::onEof()
{
	//can be zero if empty expression is allowed
	assert(_stack.size() <= 1);
}

void SyntaxTreeBuilder::onChoice()
{
	Util::onBinaryOp<Choice>(_stack);
}

void SyntaxTreeBuilder::onConcat()
{
	Util::onBinaryOp<Concat>(_stack);
}

void SyntaxTreeBuilder::onZeroOrOne()
{
	Util::onUnaryOp<ZeroOrOne>(_stack);
}

void SyntaxTreeBuilder::onZeroToMany()
{
	Util::onUnaryOp<ZeroToMany>(_stack);
}

void SyntaxTreeBuilder::onOneToMany()
{
	Util::onUnaryOp<OneToMany>(_stack);
}

void SyntaxTreeBuilder::onSymbol(const regex::REToken& t_)
{
	Util::push(_stack, new ast::AcceptorImpl<Symbol>(t_._lexeme));
}

void SyntaxTreeBuilder::onCharClass()
{
	Util::onUnaryOp<CharClass>(_stack);
}

void SyntaxTreeBuilder::onNegate()
{
	Util::onUnaryOp<Negate>(_stack);
}

void SyntaxTreeBuilder::onRngConcat()
{
	Util::onBinaryOp<RngConcat>(_stack);
}

void SyntaxTreeBuilder::onRng()
{
	Util::onBinaryOp<Rng>(_stack);
}

void SyntaxTreeBuilder::onCharClassSymbol(const regex::CCToken& t_)
{
	Util::push(_stack, new ast::AcceptorImpl<CharClassSymbol>(t_._lexeme));
}

}}
