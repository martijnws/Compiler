#include "NFABuilderVisitor.h"
#include <SyntaxTreeLib/SyntaxNode.h>
#include <cassert>

namespace mws {

NFABuilderVisitor::NFABuilderVisitor(const std::set<RangeKey>& rkSet_)
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

	// TODO: verify the stronger assertion that all nfas have have empty transitionMaps for their final state.

    // E transition from rhs.f into lhs.final should not open the door to E transition back loop
    assert(lhs._f->_transitionMap.find(NFA::E) == lhs._f->_transitionMap.end());

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

void NFABuilderVisitor::visit(const ast::ZeroOrOne& n_)
{
    n_.opr().accept(*this);
    auto opr = _result;
    
    // skipping opr (zero repition) should not open the door to E transition loop backs
    assert(opr._f->_transitionMap.find(NFA::E) == opr._f->_transitionMap.end());

    // opr.start ->(e) opr.final (for zero repition) 
    opr._s->_transitionMap.insert(std::make_pair(NFA::E, opr._f));
    
    _result = opr;
}

void NFABuilderVisitor::visit(const ast::ZeroToMany& n_)
{
    // Note: composition(OnetoMany, ZeroOrOne) == ZeroToMany
    // <factor>+? == <factor>*

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

void NFABuilderVisitor::visit(const ast::OneToMany& n_)
{
	n_.opr().accept(*this);
    auto opr = _result;

    // Note: new start and final states are added to keep the loop back isolated. This ensures we
    // don't have to worry about the loop back when composing this sub NFA with other sub NFA's
    auto s = new NFANode();
    auto f = new NFANode();

    // almost the same as ZeroToMany. Only the zero repetition (start ->(e) final ) is omitted.
  
    // start ->(e) opr.start -> opr.final ->(e) final (1 repetition)
    s->_transitionMap.insert(std::make_pair(NFA::E, opr._s));
    opr._f->_transitionMap.insert(std::make_pair(NFA::E, f));

    // opr.final ->(e) opr.start (loop back)
    opr._f->_transitionMap.insert(std::make_pair(NFA::E, opr._s));

    _result._s = s;
    _result._f = f;
}


void NFABuilderVisitor::visit(const ast::CharClass& n_)
{
	CharClassVisitor ccv;

	n_.opr().accept(ccv);

    if (ccv._charClassSet.empty())
    {
        return;
    }

    auto s = new NFANode();
    auto f = new NFANode();

    for (const auto& rk : ccv._charClassSet)
    {
        std::vector<RangeKey> rkVec = getDisjointRangeKeys(_rkSet, rk);

        for (const auto& rk : rkVec)
        {
            s->_transitionMap.insert(std::make_pair(rk, f));
        }
    }

    _result._s = s;
    _result._f = f;
}

};
