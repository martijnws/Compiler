#pragma once

#include "SyntaxNodeFwd.h"
#include <SyntaxTreeLib/SyntaxNode.h>

namespace mws { namespace regex {

class Symbol
	:
	public Leaf
{
public:
	Symbol(CodePoint l_)
		: Leaf(), _l(l_)
	{
	
	}

	CodePoint lexeme() const
	{
		return _l;
	}

private:
	const CodePoint _l;
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
	CharClassSymbol(CodePoint l_)
		: Symbol(l_)
	{
	
	}
};

}}
