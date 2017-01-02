#include "ToStringVisitor.h"
#include <RegexSyntaxTreeLib/SyntaxNode.h>
#include <CommonLib/Unicode.h>

namespace mws {

void ToStringVisitor::visit(const regex::Symbol& n_)
{
	Char buf[8] = { _C('\0') };
	const auto size = common::utf::Encoder<Char>::encode(n_.lexeme(), buf);
	_result.insert(_result.end(), buf, buf + size);
}

void ToStringVisitor::visit(const regex::Choice& n_)
{
	_result += _C("(");
	n_.lhs()->accept(*this);
	_result += _C(")");
	_result += _C("|");
	_result += _C("(");
	n_.rhs()->accept(*this);
	_result += _C(")");
}

void ToStringVisitor::visit(const regex::Concat& n_)
{
	n_.lhs()->accept(*this);

	n_.rhs()->accept(*this);
}

void ToStringVisitor::visit(const regex::ZeroToMany& n_)
{
	_result += _C("(");
	n_.opr()->accept(*this);
	_result += _C(")");
	_result += _C("*");
}

void ToStringVisitor::visit(const regex::CharClass& n_)
{
	_result += _C("[");
	n_.opr()->accept(*this);
	_result += _C("]");
}

void ToStringVisitor::visit(const regex::Negate& n_)
{
	_result += _C("^");
	n_.opr()->accept(*this);
}

void ToStringVisitor::visit(const regex::RngConcat& n_)
{
	n_.lhs()->accept(*this);

	n_.rhs()->accept(*this);
}

void ToStringVisitor::visit(const regex::Rng& n_)
{
	n_.lhs()->accept(*this);
	_result += _C("-");
	n_.rhs()->accept(*this);
}

void ToStringVisitor::visit(const regex::CharClassSymbol& n_)
{
    visit(static_cast<const regex::Symbol&>(n_));
}

};
