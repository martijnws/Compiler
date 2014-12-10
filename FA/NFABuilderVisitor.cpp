#include "NFABuilderVisitor.h"
#include <SyntaxTreeLib/SyntaxNode.h>
#include <cassert>

namespace mws {

NFABuilderVisitor::NFABuilderVisitor(const std::set<RangeKey, RangeKey::Less>& rkSet_)
:
    _rkSet(rkSet_)
{
    
}
    
void NFABuilderVisitor::visit(const ast::Symbol& n_)
{
	auto s = new NFANode();
    auto f = new NFANode();

    // start ->(e) final
    s->_transitionMap.insert(std::make_pair(n_.lexeme(), f));
    _result._s = s;
    _result._f = f;
}

void NFABuilderVisitor::visit(const ast::Choice& n_)
{
    n_.lhs().accept(*this);
    auto lhs = _result;

    n_.rhs().accept(*this);
    auto rhs = _result;

    lhs._s->_transitionMap.insert(std::make_pair(NFA::E, rhs._s));
    rhs._f->_transitionMap.insert(std::make_pair(NFA::E, lhs._f));

    _result = lhs;
}

void NFABuilderVisitor::visit(const ast::Concat& n_)
{
	n_.lhs().accept(*this);
    auto lhs = _result;

    n_.rhs().accept(*this);
    auto rhs = _result;

    // lhs.start -> lhs.final -> rhs.final
    assert(lhs._f->_transitionMap.empty());
    lhs._f->_transitionMap.swap(rhs._s->_transitionMap);
    assert(rhs._s->_transitionMap.empty());
    delete rhs._s;

    _result._s = lhs._s;
    _result._f = rhs._f;
}

void NFABuilderVisitor::visit(const ast::ZeroToMany& n_)
{
	n_.opr().accept(*this);
    auto opr = _result;

    auto s = new NFANode();
    auto f = new NFANode();

    // start ->(e) opr.start -> opr.final ->(e) final (1 repetition)
    s->_transitionMap.insert(std::make_pair(NFA::E, opr._s));
    opr._f->_transitionMap.insert(std::make_pair(NFA::E, f));

    // start ->(e) final (for zero repetition)
    s->_transitionMap.insert(std::make_pair(NFA::E, f));

    // opr.final ->(e) opr.start (loop back)
    opr._f->_transitionMap.insert(std::make_pair(NFA::E, opr._s));

    _result._s = s;
    _result._f = f;
}

void NFABuilderVisitor::visit(const ast::CharClass& n_)
{
    assert(_charClassSet.empty());

	n_.opr().accept(*this);

    if (_charClassSet.empty())
    {
        return;
    }

     auto s = new NFANode();
     auto f = new NFANode();

    for (const auto& rk : _charClassSet)
    {
        std::vector<RangeKey> rkVec = getDisjointRangeKeys(_rkSet, rk);

        for (const auto& rk : rkVec)
        {
            s->_transitionMap.insert(std::make_pair(rk, f));
        }
    }

    _charClassSet.clear();

    _result._s = s;
    _result._f = f;
}

};
