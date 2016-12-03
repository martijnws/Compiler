#include "ToStringVisitor.h"
#include <SyntaxTreeLib/SyntaxNode.h>
#include <CommonLib/Unicode.h>

namespace mws {

void ToStringVisitor::visit(const ast::Symbol& n_)
{
	Char buf[8] = { _C('\0') };
	const auto size = common::utf::Encoder<Char>::encode(n_.lexeme(), buf);
	_result.insert(_result.end(), buf, buf + size);
}

void ToStringVisitor::visit(const ast::Choice& n_)
{
	_result += _C("(");
	n_.lhs().accept(*this);
	_result += _C(")");
	_result += _C("|");
	_result += _C("(");
	n_.rhs().accept(*this);
	_result += _C(")");
}

void ToStringVisitor::visit(const ast::Concat& n_)
{
	n_.lhs().accept(*this);

	n_.rhs().accept(*this);
}

void ToStringVisitor::visit(const ast::ZeroToMany& n_)
{
	_result += _C("(");
	n_.opr().accept(*this);
	_result += _C(")");
	_result += _C("*");
}

void ToStringVisitor::visit(const ast::CharClass& n_)
{
	_result += _C("[");
	n_.opr().accept(*this);
	_result += _C("]");
}

void ToStringVisitor::visit(const ast::Negate& n_)
{
	_result += _C("^");
	n_.opr().accept(*this);
}

void ToStringVisitor::visit(const ast::RngConcat& n_)
{
	n_.lhs().accept(*this);

	n_.rhs().accept(*this);
}

void ToStringVisitor::visit(const ast::Rng& n_)
{
	n_.lhs().accept(*this);
	_result += _C("-");
	n_.rhs().accept(*this);
}

void ToStringVisitor::visit(const ast::CharClassSymbol& n_)
{
    visit(static_cast<const ast::Symbol&>(n_));
}

};
