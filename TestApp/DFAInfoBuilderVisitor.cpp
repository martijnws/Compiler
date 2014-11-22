#include "DFAInfoBuilderVisitor.h"
#include "DFAInfo.h"
#include <SyntaxTreeLib/SyntaxNode.h>
#include <algorithm>
#include <iterator>
#include <cassert>

namespace {

std::set<char> sigma()
{
    std::set<char> result;
    // 255 is reserved for NFA::E
    for (int c = 0; c < 255; ++c)
    {
        result.insert(static_cast<char>(c));
    }

    return result;
}

static const std::set<char> _sigma = sigma();

}

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
    DFAInfo* lhs = _dfaInfo;

    n_.rhs().accept(*this);
    DFAInfo* rhs = _dfaInfo;
  
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
    DFAInfo* lhs = _dfaInfo;

    n_.rhs().accept(*this);
    DFAInfo* rhs = _dfaInfo;
  
    // followPos calculates followPos of the subtrees, not of the current node.
    for (DFAInfo* dfaInfo : lhs->_lastPos)
    {
        dfaInfo->_followPos.insert(rhs->_firstPos.begin(), rhs->_firstPos.end());
    }

    auto n = new DFAInfo();

    n->_isNullable = lhs->_isNullable && rhs->_isNullable;

    if (lhs->_isNullable)
    {
        std::set_union(
            lhs->_firstPos.begin(), lhs->_firstPos.end(), 
            rhs->_firstPos.begin(), rhs->_firstPos.end(), 
            std::inserter(n->_firstPos, n->_firstPos.begin()));
    }
    else
    {
        n->_firstPos = lhs->_firstPos;
    }

    if (rhs->_isNullable)
    {
        std::set_union(
            lhs->_lastPos.begin(), lhs->_lastPos.end(), 
            rhs->_lastPos.begin(), rhs->_lastPos.end(), 
            std::inserter(n->_lastPos, n->_lastPos.begin()));
    }
    else
    {
        n->_lastPos = rhs->_lastPos;
    }

    _dfaInfo = n;
}

void DFAInfoBuilderVisitor::visit(const ast::ZeroToMany& n_)
{
	n_.opr().accept(*this);
    DFAInfo* opr = _dfaInfo;

    // followPos calculates followPos of the subtrees, not of the current node.
    for (DFAInfo* dfaInfo : opr->_lastPos)
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
    DFAInfo* opr = _dfaInfo;

    auto n = new DFAInfo();

    n->_isNullable = _charClassSet.empty();

    for (char c : _charClassSet)
    {
        ast::AcceptorImpl<ast::Symbol> symbol(c);
        visit(symbol);
       
        n->_firstPos.insert(_dfaInfo);
        n->_lastPos.insert(_dfaInfo);
    }

    _dfaInfo = n;
}

void DFAInfoBuilderVisitor::visit(const ast::Negate& n_)
{
    n_.opr().accept(*this);

    std::set<char> complement;
    auto out = std::inserter(complement, complement.begin());
    std::set_difference(_sigma.begin(), _sigma.end(), _charClassSet.begin(), _charClassSet.end(), out);
    _charClassSet.swap(complement);
}

void DFAInfoBuilderVisitor::visit(const ast::RngConcat& n_)
{
	n_.lhs().accept(*this);
    n_.rhs().accept(*this);
}

void DFAInfoBuilderVisitor::visit(const ast::Rng& n_)
{
	for (char c = n_.lhsSymbol().lexeme(); c < n_.rhsSymbol().lexeme(); ++c)
    {
        _charClassSet.insert(c);
    }
}

void DFAInfoBuilderVisitor::visit(const ast::CharClassSymbol& n_)
{
    _charClassSet.insert(n_.lexeme());
}

};
