#pragma once

#include "RangeKey.h"
#include <SyntaxTreeLib/Visitor.h>
#include <set>

namespace mws {

class DFAInfo;

class FABuilderVisitorBase
	:
	public mws::ast::Visitor
{
public:
	virtual void visit(const ast::Negate& n_);
    virtual void visit(const ast::RngConcat& n_);
	virtual void visit(const ast::Rng& n_);
    virtual void visit(const ast::CharClassSymbol& n_);

protected:
    std::set<RangeKey, RangeKey::Less> _charClassSet;
};

}


