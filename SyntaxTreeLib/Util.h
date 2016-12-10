#pragma once

#include <SyntaxTreeLib/SyntaxNode.h>
#include <stack>
#include <cassert>

namespace mws { namespace ast { 

template<typename SyntaxNode, typename Stack>
class Util
{
public:
	static void push(Stack& stack_, SyntaxNode* n_)
	{
		stack_.push(n_);
	}

	static SyntaxNode* pop(Stack& stack_)
	{
		assert(!stack_.empty());

		auto n = stack_.top();
		stack_.pop();
		return n;
	}

	static SyntaxNode* popLast(Stack& stack_)
	{
		if (stack_.size())
		{
			assert(stack_.size() == 1);
			return Util::pop(stack_);
		}
		else
		{
			return nullptr;
		}
	}

	template<typename OperatorT>
	static void onUnaryOp(Stack& stack_)
	{
		auto n = Util::pop(stack_);
		Util::push(stack_, new ast::AcceptorImpl<OperatorT>(n));
	}

	template<typename OperatorT>
	static void onBinaryOp(Stack& stack_)
	{
		auto rhs = Util::pop(stack_);
		auto lhs = Util::pop(stack_);
		Util::push(stack_, new ast::AcceptorImpl<OperatorT>(lhs, rhs));
	}
};

}}
