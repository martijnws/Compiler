#pragma once

#include <Grammar/Token.h>
#include <memory>

namespace mws { namespace ast {

template<typename V>
class Acceptor
{
public:
	using Visitor = V;

    virtual ~Acceptor() 
	{

	}
	virtual void accept(Visitor& visitor_) const = 0;
};

template<typename Base>
class AcceptorImpl
	:
	public Base
{
public:
	using Visitor = typename Base::Visitor;

	template<typename... Args>
	AcceptorImpl(Args... args_)
		:
		Base(args_...)
	{
	
	}

	void accept(Visitor& visitor_) const override
	{
		visitor_.visit(static_cast<const Base&>(*this));
	}
};

template<typename V>
class SyntaxNode
	:
	public Acceptor<V>
{
public:
	using Ptr = std::shared_ptr<const SyntaxNode<V>>;

	SyntaxNode(const SyntaxNode&) = delete;
	SyntaxNode& operator = (const SyntaxNode&) = delete;

protected:
	SyntaxNode() = default;
};

template<typename V>
class Leaf
	:
	public SyntaxNode<V>
{
public:
	Leaf(const Leaf&) = delete;
	Leaf& operator = (const Leaf&) = delete;

protected:
	Leaf() = default;
};

template<typename V>
class UnaryOp
	:
	public SyntaxNode<V>
{
protected:
	UnaryOp(SyntaxNode<V>::Ptr n_)
		: _n(n_)
	{
	
	}

public:

	const SyntaxNode<V>::Ptr& opr() const
	{
		return _n;
	}

private:
	SyntaxNode<V>::Ptr _n;
};

template<typename V>
class BinaryOp
	:
	public SyntaxNode<V>
{
protected:

	BinaryOp(SyntaxNode<V>::Ptr lhs_, SyntaxNode<V>::Ptr rhs_)
		: _lhs(lhs_), _rhs(rhs_)
	{
	
	}

public:

	const SyntaxNode<V>::Ptr& lhs() const
	{
		return _lhs;
	}

	const SyntaxNode<V>::Ptr& rhs() const
	{
		return _rhs;
	}

private:
	SyntaxNode<V>::Ptr _lhs;
	SyntaxNode<V>::Ptr _rhs;
};

template<typename V>
class TernaryOp
	:
	public SyntaxNode<V>
{
protected:

	TernaryOp(SyntaxNode<V>::Ptr op0_, SyntaxNode<V>::Ptr op1_, SyntaxNode<V>::Ptr op2_)
		: _op0(op0_), _op1(op1_), _op2(op2_)
	{
	
	}

public:

	const SyntaxNode<V>::Ptr& op0() const
	{
		return _op0;
	}

	const SyntaxNode<V>::Ptr& op1() const
	{
		return _op1;
	}

	const SyntaxNode<V>::Ptr& op2() const
	{
		return _op2;
	}

private:
	SyntaxNode<V>::Ptr _op0;
	SyntaxNode<V>::Ptr _op1;
	SyntaxNode<V>::Ptr _op2;
};

}}
