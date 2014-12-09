#include "DFAInfoBuilderVisitor.h"
#include "DFAInfo.h"
#include <SyntaxTreeLib/SyntaxNode.h>
#include <algorithm>
#include <iterator>
#include <cassert>

namespace mws {

void DFAInfoBuilderVisitor::visit(const ast::Symbol& n_)
{
    auto n = new DFAInfo();

    n->_isNullable = false;
    n->_firstPos.insert(n);
    n->_lastPos.insert(n);

    n->_lexeme = n_.lexeme();

    _dfaInfo = n;
}

void DFAInfoBuilderVisitor::visit(const ast::Choice& n_)
{
    n_.lhs().accept(*this);
    auto lhs = _dfaInfo;

    n_.rhs().accept(*this);
    auto rhs = _dfaInfo;
  
    auto n = new DFAInfo();

    n->_isNullable = lhs->_isNullable || rhs->_isNullable;

    std::set_union(
        lhs->_firstPos.begin(), lhs->_firstPos.end(), 
        rhs->_firstPos.begin(), rhs->_firstPos.end(), 
        std::inserter(n->_firstPos, n->_firstPos.begin()));

    std::set_union(
        lhs->_lastPos.begin(), lhs->_lastPos.end(), 
        rhs->_lastPos.begin(), rhs->_lastPos.end(), 
        std::inserter(n->_lastPos, n->_lastPos.begin()));

    _dfaInfo = n;
}

void DFAInfoBuilderVisitor::visit(const ast::Concat& n_)
{
	n_.lhs().accept(*this);
    auto lhs = _dfaInfo;

    n_.rhs().accept(*this);
    auto rhs = _dfaInfo;

    _dfaInfo = concat(lhs, rhs);
}

DFAInfo* DFAInfoBuilderVisitor::concat(const DFAInfo* lhs_, const DFAInfo* rhs_) const
{
    // followPos calculates followPos of the subtrees, not of the current node.
    for (auto dfaInfo : lhs_->_lastPos)
    {
        dfaInfo->_followPos.insert(rhs_->_firstPos.begin(), rhs_->_firstPos.end());
    }

    auto n = new DFAInfo();

    n->_isNullable = lhs_->_isNullable && rhs_->_isNullable;

    if (lhs_->_isNullable)
    {
        std::set_union(
            lhs_->_firstPos.begin(), lhs_->_firstPos.end(), 
            rhs_->_firstPos.begin(), rhs_->_firstPos.end(), 
            std::inserter(n->_firstPos, n->_firstPos.begin()));
    }
    else
    {
        n->_firstPos = lhs_->_firstPos;
    }

    if (rhs_->_isNullable)
    {
        std::set_union(
            lhs_->_lastPos.begin(), lhs_->_lastPos.end(), 
            rhs_->_lastPos.begin(), rhs_->_lastPos.end(), 
            std::inserter(n->_lastPos, n->_lastPos.begin()));
    }
    else
    {
        n->_lastPos = rhs_->_lastPos;
    }

    return n;
}

void DFAInfoBuilderVisitor::visit(const ast::ZeroToMany& n_)
{
	n_.opr().accept(*this);
    auto opr = _dfaInfo;

    // followPos calculates followPos of the subtrees, not of the current node.
    for (auto dfaInfo : opr->_lastPos)
    {
        dfaInfo->_followPos.insert(opr->_firstPos.begin(), opr->_firstPos.end());
    }

    auto n = new DFAInfo();

    n->_isNullable = true;

    n->_firstPos = opr->_firstPos;
    n->_lastPos = opr->_lastPos;

    _dfaInfo = n;
}

void DFAInfoBuilderVisitor::visit(const ast::CharClass& n_)
{
    assert(_charClassSet.empty());

	n_.opr().accept(*this);
    
    auto ncs = new DFAInfo();

    ncs->_isNullable = _charClassSet.empty();

    for (const auto& rk : _charClassSet)
    {
        // TODO: implement properly
        for (auto c = rk._l; c <= rk._h; ++c)
        {
            auto n = new DFAInfo();

            n->_isNullable = false;
            n->_firstPos.insert(n);
            n->_lastPos.insert(n);

            n->_lexeme = RangeKey(c, c);
       
            ncs->_firstPos.insert(n);
            ncs->_lastPos.insert(n);
        }
    }

    _dfaInfo = ncs;
}

DFAInfo* DFAInfoBuilderVisitor::startState() const
{ 
    return concat(_dfaInfo, acceptState());
}

DFAInfo* DFAInfoBuilderVisitor::acceptState() const
{
    static DFAInfo n;
    n._isNullable = true;
    n._firstPos.insert(&n);
    n._lastPos.insert(&n);
    n._lexeme = '#';
    return &n;
}

};
