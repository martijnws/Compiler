#pragma once

#include "FABuilderVisitorBase.h"

namespace mws {

class DFAInfo;

class DFAInfoBuilderVisitor 
	:
	public FABuilderVisitorBase
{
public:
    DFAInfoBuilderVisitor(const std::set<RangeKey, RangeKey::Less>& rkSet_);

	virtual void visit(const ast::Symbol& n_);
	virtual void visit(const ast::Choice& n_);
	virtual void visit(const ast::Concat& n_);
	virtual void visit(const ast::ZeroToMany& n_);
	virtual void visit(const ast::CharClass& n_);
	
    DFAInfo* startState() const;
    DFAInfo* acceptState() const;

private:
    DFAInfo* concat(const DFAInfo* lhs_, const DFAInfo* rhs_) const;

private:
    DFAInfo* _dfaInfo;
    std::set<RangeKey, RangeKey::Less> _rkSet;
};

}

