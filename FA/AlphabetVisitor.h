#pragma once

#include "CharClassVisitor.h"
#include "RangeKey.h"
#include <vector>

namespace mws {

class AlphabetVisitor 
	:
	public regex::Visitor
{
public:
	void visit(const regex::Symbol& n_) override;
    void visit(const regex::UnaryOp& n_) override;
    void visit(const regex::BinaryOp& n_) override;
	void visit(const regex::CharClass& n_) override;

    std::vector<RangeKey> _rkVec;
};

};
