#pragma once

#include "CharClassVisitor.h"
#include "RangeKey.h"
#include <vector>

namespace mws {

class AlphabetVisitor 
	:
	public ast::Visitor
{
public:
	virtual void visit(const ast::Symbol& n_);
    virtual void visit(const ast::UnaryOp& n_);
    virtual void visit(const ast::BinaryOp& n_);
	virtual void visit(const ast::CharClass& n_);

    std::vector<RangeKey> _rkVec;
};

};
