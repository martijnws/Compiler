#pragma once

#include <SyntaxTreeLib/Visitor.h>
#include <set>

namespace mws {

class DFAInfo;

class DFAInfoBuilderVisitor 
	:
	public mws::ast::Visitor
{
public:
	virtual void visit(const ast::Symbol& n_);
	virtual void visit(const ast::Choice& n_);
	virtual void visit(const ast::Concat& n_);
	virtual void visit(const ast::ZeroToMany& n_);
	virtual void visit(const ast::CharClass& n_);
	virtual void visit(const ast::Negate& n_);
    virtual void visit(const ast::RngConcat& n_);
	virtual void visit(const ast::Rng& n_);
    virtual void visit(const ast::CharClassSymbol& n_);

    DFAInfo* startState() const;
    DFAInfo* acceptState() const;

private:
    DFAInfo* concat(const DFAInfo* lhs_, const DFAInfo* rhs_) const;

private:
    std::set<char> _charClassSet;
    DFAInfo*       _dfaInfo;
};

}

