#include "CharClassVisitor.h"
#include "NFA.h"
#include <RegexSyntaxTreeLib/SyntaxNode.h>
#include <cassert>

namespace mws {

void CharClassVisitor::visit(const regex::Negate& n_)
{
    n_.opr().accept(*this);

    std::set<RangeKey> complement;
    CodePoint l = 0;
    // insert artificial upperbound so that complement up to (but not including) upperbound will be inserted in complement
    _charClassSet.insert(NFA::E);
    for (const auto& rk : _charClassSet)
    {
        if (rk._l > l)
        {
            auto res = complement.insert(RangeKey(l, rk._l - 1));
            assert(res.second);
        }

        l = rk._h + 1;
    }

    _charClassSet.swap(complement);
}

void CharClassVisitor::visit(const regex::RngConcat& n_)
{
	n_.lhs().accept(*this);
    n_.rhs().accept(*this);
}

void CharClassVisitor::visit(const regex::Rng& n_)
{
    RangeKey rkNew(n_.lhsSymbol().lexeme(), n_.rhsSymbol().lexeme());

    std::vector<RangeKey> rkVec;
    rkVec.push_back(rkNew);

    // find all overlapping ranges (non disjoint sets are considered equal)
    for (auto itr = _charClassSet.find(rkNew); itr != _charClassSet.end(); itr = _charClassSet.find(rkNew))
    {
        rkVec.push_back(*itr);
        _charClassSet.erase(itr);
    }

    // re insert disjoint ranges
    const auto rkSet = getDisjointRangeSet(rkVec);
    for (const auto& rk : rkSet)
    {
        auto res = _charClassSet.insert(rk);
        assert(res.second);
    }
}

void CharClassVisitor::visit(const regex::CharClassSymbol& n_)
{
    _charClassSet.insert(n_.lexeme());
}

};
