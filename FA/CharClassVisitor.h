#pragma once

#include "RangeKey.h"
#include <RegexSyntaxTreeLib/Visitor.h>
#include <set>

namespace mws {

class DFAInfo;

class CharClassVisitor
	:
	public regex::Visitor
{
public:
	void visit(const regex::Negate& n_) override;
    void visit(const regex::RngConcat& n_) override;
	void visit(const regex::Rng& n_) override;
    void visit(const regex::CharClassSymbol& n_) override;

    std::set<RangeKey> _charClassSet;
};

}


