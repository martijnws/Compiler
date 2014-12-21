#pragma once

#include "Visitor.h"
#include <Grammar/Token.h>
#include <memory>

namespace mws { namespace ast {

class SyntaxNode;
using SyntaxNodePtr = std::unique_ptr<const SyntaxNode>;

class Acceptor
{
public:
    virtual ~Acceptor() {}
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
	Symbol(char l_)
		: SyntaxNode(), _l(l_)
	{
	
	}

	char lexeme() const
	{
		return _l;
	}

private:
	const char _l;
};

class UnaryOp
	:
	public SyntaxNode
{
protected:
  
	UnaryOp(const SyntaxNode* n_)
		: _n(n_)
	{
	
	}

public:

	const SyntaxNode& opr() const
	{
		return *_n;
	}

private:
	SyntaxNodePtr _n;
};

class BinaryOp
	:
	public SyntaxNode
{
protected:

	BinaryOp(const SyntaxNode* lhs_, const SyntaxNode* rhs_)
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
	SyntaxNodePtr _lhs;
	SyntaxNodePtr _rhs;
};

class Choice
	:
	public BinaryOp
{
public:
	Choice(const SyntaxNode* lhs_, const SyntaxNode* rhs_)
		: BinaryOp(lhs_, rhs_)
	{
	
	}
};

class Concat
	:
	public BinaryOp
{
public:
	Concat(const SyntaxNode* lhs_, const SyntaxNode* rhs_)
		: BinaryOp(lhs_, rhs_)
	{
	
	}
};

class ZeroOrOne
	:
	public UnaryOp
{
public:
	ZeroOrOne(const SyntaxNode* n_)
		: UnaryOp(n_)
	{
	
	}
};

class ZeroToMany
	:
	public UnaryOp
{
public:
	ZeroToMany(const SyntaxNode* n_)
		: UnaryOp(n_)
	{
	
	}
};

class OneToMany
	:
	public UnaryOp
{
public:
	OneToMany(const SyntaxNode* n_)
		: UnaryOp(n_)
	{
	
	}
};


class CharClass
	:
	public UnaryOp
{
public:
	CharClass(const SyntaxNode* n_)
		: UnaryOp(n_)
	{
	
	}
};

class Negate
	:
	public UnaryOp
{
public:
	Negate(const SyntaxNode* n_)
		: UnaryOp(n_)
	{
	
	}
};

class RngConcat
	:
	public BinaryOp
{
public:
	RngConcat(const SyntaxNode* lhs_, const SyntaxNode* rhs_)
		: BinaryOp(lhs_, rhs_)
	{
	
	}
};

class Rng
	:
	public BinaryOp
{
public:
	Rng(const SyntaxNode* lhs_, const SyntaxNode* rhs_)
		: BinaryOp(lhs_, rhs_)
	{
	
	}

    const Symbol& lhsSymbol() const
    {
        return static_cast<const Symbol&>(lhs());
    }

    const Symbol& rhsSymbol() const
    {
        return static_cast<const Symbol&>(rhs());
    }
};

class CharClassSymbol
	:
	public Symbol
{
public:
	CharClassSymbol(char l_)
		: Symbol(l_)
	{
	
	}
};

}}
