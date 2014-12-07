#include "NFABuilderVisitor.h"
#include <SyntaxTreeLib/SyntaxNode.h>
#include <SyntaxTreeLib/SyntaxTreeBuilder.h>
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

    auto s = new NFANode();
    auto f = new NFANode();

    // start ->(e) lhs.start -> lhs.final ->(e) final
    s->_transitionMap.insert(std::make_pair(NFA::E, lhs._s));
    lhs._f->_transitionMap.insert(std::make_pair(NFA::E, f));

    // start ->(e) rhs.start -> rhs.final ->(e) final
    s->_transitionMap.insert(std::make_pair(NFA::E, rhs._s));
    rhs._f->_transitionMap.insert(std::make_pair(NFA::E, f));

    _result._s = s;
    _result._f = f;
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

// Very slow CharClass NFA. Due to long E paths, conversion to DFA (think e_closure, DFANode's with lots
// of NFANode's with only E in and out edges) is very slow
void NFABuilderVisitor::visit(const ast::CharClass& n_)
{
    assert(_charClassSet.empty());

	n_.opr().accept(*this);

    if (_charClassSet.empty())
    {
        return;
    }

    // replace CharClass with Concatenation of Choice nodes
    ast::SyntaxTreeBuilder astBuilder;

    auto itr = _charClassSet.begin();
    astBuilder.onSymbol(td::LL1::Token{td::LL1::Token::Symbol, *itr});
    ++itr;

    for ( ; itr != _charClassSet.end(); ++itr)
    {
        // push second argument
        astBuilder.onSymbol(td::LL1::Token{td::LL1::Token::Symbol, *itr});
        // pop 2 arguments, push back 1 choice argument
        astBuilder.onChoice();
    }

    _charClassSet.clear();

    // continue NFA construction from temporarely created AST
    ast::SyntaxNodePtr root(astBuilder.detach());
    root->accept(*this);
}

void NFABuilderVisitor::visit(const ast::Negate& n_)
{
    n_.opr().accept(*this);

    std::set<char> complement;
    auto out = std::inserter(complement, complement.begin());
    std::set_difference(_sigma.begin(), _sigma.end(), _charClassSet.begin(), _charClassSet.end(), out);
    _charClassSet.swap(complement);
}

void NFABuilderVisitor::visit(const ast::RngConcat& n_)
{
	n_.lhs().accept(*this);
    n_.rhs().accept(*this);
}

void NFABuilderVisitor::visit(const ast::Rng& n_)
{
	for (auto c = n_.lhsSymbol().lexeme(); c < n_.rhsSymbol().lexeme(); ++c)
    {
        _charClassSet.insert(c);
    }
}

void NFABuilderVisitor::visit(const ast::CharClassSymbol& n_)
{
    _charClassSet.insert(n_.lexeme());
}

};
