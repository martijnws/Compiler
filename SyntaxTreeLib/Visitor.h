#pragma once

namespace mws { namespace ast {

class Symbol;
class UnaryOp;
class BinaryOp;
class Choice;
class Concat;
class ZeroToMany;
class CharClass;
class Negate;
class RngConcat;
class Rng;
class CharClassSymbol;

class Visitor
{
public:
	virtual ~Visitor(){}

	virtual void visit(const Symbol& n_) = 0;
	virtual void visit(const Choice& n_) = 0;
	virtual void visit(const Concat& n_) = 0;
	virtual void visit(const ZeroToMany& n_) = 0;
	virtual void visit(const CharClass& n_) = 0;
	virtual void visit(const Negate& n_) = 0;
    virtual void visit(const RngConcat& n_) = 0;
	virtual void visit(const Rng& n_) = 0;
    virtual void visit(const CharClassSymbol& n_) = 0;
};

}}