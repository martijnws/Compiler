#pragma once

#include <Grammar/Token.h>
#include <memory>

namespace mws { namespace ast {

template<typename V>
class Acceptor
{
public:
	using Visitor = V;

    virtual ~Acceptor() {}
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
	using Ptr = std::unique_ptr<const SyntaxNode<V>>;

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
	UnaryOp(const SyntaxNode<V>* n_)
		: _n(n_)
	{
	
	}

public:

	const SyntaxNode<V>& opr() const
	{
		return *_n;
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

	BinaryOp(const SyntaxNode<V>* lhs_, const SyntaxNode<V>* rhs_)
		: _lhs(lhs_), _rhs(rhs_)
	{
	
	}

public:

	const SyntaxNode<V>& lhs() const
	{
		return *_lhs;
	}

	const SyntaxNode<V>& rhs() const
	{
		return *_rhs;
	}

private:
	SyntaxNode<V>::Ptr _lhs;
	SyntaxNode<V>::Ptr _rhs;
};

}}
