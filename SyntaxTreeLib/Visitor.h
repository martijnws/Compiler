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

	virtual void visit(const Symbol& n_) {};
	virtual void visit(const Choice& n_) {};
	virtual void visit(const Concat& n_) {};
	virtual void visit(const ZeroToMany& n_) {};
	virtual void visit(const CharClass& n_) {};
	virtual void visit(const Negate& n_) {};
    virtual void visit(const RngConcat& n_) {};
	virtual void visit(const Rng& n_) {};
    virtual void visit(const CharClassSymbol& n_) {};
};

}}