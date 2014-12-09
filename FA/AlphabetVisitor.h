#pragma once

#include "FABuilderVisitorBase.h"
#include "RangeKey.h"
#include <vector>

namespace mws {

class AlphabetVisitor 
	:
	public FABuilderVisitorBase
{
public:
	virtual void visit(const ast::Symbol& n_);
	virtual void visit(const ast::Choice& n_);
	virtual void visit(const ast::Concat& n_);
	virtual void visit(const ast::ZeroToMany& n_);
	virtual void visit(const ast::CharClass& n_);

    std::vector<RangeKey> _rkVec;
};

};
