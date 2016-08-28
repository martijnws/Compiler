#pragma once

#include <SyntaxTreeLib\Visitor.h>

class ToStringVisitor
:
	public mws::ast::Visitor
{
public:
	void visit(const mws::ast::Symbol& n_) override;
	void visit(const mws::ast::Choice& n_) override; 
	void visit(const mws::ast::Concat& n_) override;
    void visit(const mws::ast::ZeroOrOne& n_) override;
	void visit(const mws::ast::ZeroToMany& n_) override;
    void visit(const mws::ast::OneToMany& n_) override;
	void visit(const mws::ast::CharClass& n_) override;
	void visit(const mws::ast::Negate& n_) override;
    void visit(const mws::ast::RngConcat& n_) override;
	void visit(const mws::ast::Rng& n_) override;
    void visit(const mws::ast::CharClassSymbol& n_) override;

	std::wstring m_result;
};
