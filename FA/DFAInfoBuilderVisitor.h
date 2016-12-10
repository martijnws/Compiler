#pragma once

#include "CharClassVisitor.h"

namespace mws {

class DFAInfo;

class DFAInfoBuilderVisitor 
	:
	public regex::Visitor
{
public:
    DFAInfoBuilderVisitor(const std::set<RangeKey>& rkSet_);

	void visit(const regex::Symbol& n_) override;
	void visit(const regex::Choice& n_) override;
	void visit(const regex::Concat& n_) override;
	void visit(const regex::ZeroToMany& n_) override;
	void visit(const regex::CharClass& n_) override;
	
    DFAInfo* startState() const;
    DFAInfo* acceptState() const;

private:
    DFAInfo* concat(const DFAInfo* lhs_, const DFAInfo* rhs_) const;

private:
    DFAInfo* _dfaInfo;
    const std::set<RangeKey> _rkSet;
};

}

