#include "AlphabetVisitor.h"

#include "AlphabetVisitor.h"
#include <RegexSyntaxTreeLib/SyntaxNode.h>

namespace mws {

void AlphabetVisitor::visit(const regex::Symbol& n_)
{
	_rkVec.push_back(n_.lexeme());
}

void AlphabetVisitor::visit(const regex::BinaryOp& n_)
{
	n_.lhs().accept(*this);
	n_.rhs().accept(*this);
}

void AlphabetVisitor::visit(const regex::UnaryOp& n_)
{
	n_.opr().accept(*this);
}

void AlphabetVisitor::visit(const regex::CharClass& n_)
{
	CharClassVisitor ccv;

	n_.opr().accept(ccv);

    for (const auto& rk : ccv._charClassSet)
    {
        _rkVec.push_back(rk);
    }
}

};
