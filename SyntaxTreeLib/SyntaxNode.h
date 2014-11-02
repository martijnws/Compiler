#pragma once

#include "Visitor.h"
#include <RegexLL1ParserLib/Token.h>

namespace mws { namespace ast {

class Acceptor
{
public:
	virtual void accept(Visitor& visitor_) const = 0;
};

template<typename Base>
class AcceptorImpl
	:
	public Base
{
public:

	template<typename... Args>
	AcceptorImpl(Args... args_)
		:
		Base(args_...)
	{
	
	}

	virtual void accept(Visitor& visitor_) const
	{
		visitor_.visit(static_cast<const Base&>(*this));
	}
};

class SyntaxNode
	:
	public Acceptor
{
public:
	SyntaxNode(const SyntaxNode&) = delete;
	SyntaxNode& operator = (const SyntaxNode&) = delete;

protected:
	SyntaxNode() = default;
};

class Symbol
	:
	public SyntaxNode
{
public:
	Symbol(const td::LL1::Token& t_)
		: SyntaxNode(), _t(t_)
	{
	
	}

	const td::LL1::Token& token() const
	{
		return _t;
	}

private:
	const td::LL1::Token _t;
};

class UnaryOp
	:
	public SyntaxNode
{
protected:

	UnaryOp(SyntaxNode* n_)
		: _n(n_)
	{
	
	}

public:

	const SyntaxNode& opr() const
	{
		return *_n;
	}

private:
	SyntaxNode* _n;
};

class BinaryOp
	:
	public SyntaxNode
{
protected:

	BinaryOp(SyntaxNode* lhs_, SyntaxNode* rhs_)
		: _lhs(lhs_), _rhs(rhs_)
	{
	
	}

public:

	const SyntaxNode& lhs() const
	{
		return *_lhs;
	}

	const SyntaxNode& rhs() const
	{
		return *_rhs;
	}

private:
	SyntaxNode* _lhs;
	SyntaxNode* _rhs;
};

class Choice
	:
	public BinaryOp
{
public:
	Choice(SyntaxNode* lhs_, SyntaxNode* rhs_)
		: BinaryOp(lhs_, rhs_)
	{
	
	}
};

class Concat
	:
	public BinaryOp
{
public:
	Concat(SyntaxNode* lhs_, SyntaxNode* rhs_)
		: BinaryOp(lhs_, rhs_)
	{
	
	}
};

class ZeroToMany
	:
	public UnaryOp
{
public:
	ZeroToMany(SyntaxNode* n_)
		: UnaryOp(n_)
	{
	
	}
};

class CharClass
	:
	public UnaryOp
{
public:
	CharClass(SyntaxNode* n_)
		: UnaryOp(n_)
	{
	
	}
};

class Negate
	:
	public UnaryOp
{
public:
	Negate(SyntaxNode* n_)
		: UnaryOp(n_)
	{
	
	}
};

class Range
	:
	public BinaryOp
{
public:
	Range(SyntaxNode* lhs_, SyntaxNode* rhs_)
		: BinaryOp(lhs_, rhs_)
	{
	
	}
};

}}
