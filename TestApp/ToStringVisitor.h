#pragma once

#include <SyntaxTreeLib/Visitor.h>
#include <string>

namespace mws {

class ToStringVisitor 
	:
	public mws::ast::Visitor
{
public:
	virtual void visit(const ast::Symbol& n_);
	virtual void visit(const ast::UnaryOp& n_);
	virtual void visit(const ast::BinaryOp& n_);
	virtual void visit(const ast::Choice& n_);
	virtual void visit(const ast::Concat& n_);
	virtual void visit(const ast::ZeroToMany& n_);
	virtual void visit(const ast::CharClass& n_);
	virtual void visit(const ast::Negate& n_);
	virtual void visit(const ast::Range& n_);

	std::string _result;
};

};
