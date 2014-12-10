#include "AlphabetVisitor.h"

#include "AlphabetVisitor.h"
#include <SyntaxTreeLib/SyntaxNode.h>

namespace mws {

void AlphabetVisitor::visit(const ast::Symbol& n_)
{
	_rkVec.push_back(n_.lexeme());
}

void AlphabetVisitor::visit(const ast::Choice& n_)
{
	n_.lhs().accept(*this);
	n_.rhs().accept(*this);
}

void AlphabetVisitor::visit(const ast::Concat& n_)
{
	n_.lhs().accept(*this);
	n_.rhs().accept(*this);
}

void AlphabetVisitor::visit(const ast::ZeroOrOne& n_)
{
	n_.opr().accept(*this);
}

void AlphabetVisitor::visit(const ast::ZeroToMany& n_)
{
	n_.opr().accept(*this);
}

void AlphabetVisitor::visit(const ast::OneToMany& n_)
{
	n_.opr().accept(*this);
}

void AlphabetVisitor::visit(const ast::CharClass& n_)
{
	n_.opr().accept(*this);

    for (const auto& rk : _charClassSet)
    {
        _rkVec.push_back(rk);
    }
}

};
