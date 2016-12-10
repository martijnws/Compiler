#pragma once

#include "SyntaxNodeFwd.h"

namespace mws { namespace regex {

class Visitor
{
public:
	virtual ~Visitor(){}

	virtual void visit(const Symbol& n_);
	virtual void visit(const Choice& n_); 
	virtual void visit(const Concat& n_);
    virtual void visit(const ZeroOrOne& n_);
	virtual void visit(const ZeroToMany& n_);
    virtual void visit(const OneToMany& n_);
	virtual void visit(const CharClass& n_);
	virtual void visit(const Negate& n_);
    virtual void visit(const RngConcat& n_);
	virtual void visit(const Rng& n_);
    virtual void visit(const CharClassSymbol& n_);
    virtual void visit(const Leaf& n_);
    virtual void visit(const UnaryOp& n_);
    virtual void visit(const BinaryOp& n_);
};

}} //mws::regex