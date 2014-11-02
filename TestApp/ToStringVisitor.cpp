#include "ToStringVisitor.h"
#include <SyntaxTreeLib/SyntaxNode.h>

namespace mws {

void ToStringVisitor::visit(const ast::Symbol& n_)
{
	_result += n_.token()._lexeme;
}

void ToStringVisitor::visit(const ast::UnaryOp& n_)
{
	
}

void ToStringVisitor::visit(const ast::BinaryOp& n_)
{

}

void ToStringVisitor::visit(const ast::Choice& n_)
{
	_result += "(";
	n_.lhs().accept(*this);
	_result += ")";
	_result += "|";
	_result += "(";
	n_.rhs().accept(*this);
	_result += ")";
}

void ToStringVisitor::visit(const ast::Concat& n_)
{
	n_.lhs().accept(*this);

	n_.rhs().accept(*this);
}

void ToStringVisitor::visit(const ast::ZeroToMany& n_)
{
	_result += "(";
	n_.opr().accept(*this);
	_result += ")";
	_result += "*";
}

void ToStringVisitor::visit(const ast::CharClass& n_)
{
	_result += "[";
	n_.opr().accept(*this);
	_result += "]";
}

void ToStringVisitor::visit(const ast::Negate& n_)
{
	_result += "^";
	n_.opr().accept(*this);
}

void ToStringVisitor::visit(const ast::Range& n_)
{
	n_.lhs().accept(*this);
	_result += "-";
	n_.rhs().accept(*this);
}


};
