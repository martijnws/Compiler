#pragma once

#include <RegexSyntaxTreeLib\Visitor.h>

class ToStringVisitor
:
	public mws::regex::Visitor
{
public:
	void visit(const mws::regex::Symbol& n_) override;
	void visit(const mws::regex::Choice& n_) override; 
	void visit(const mws::regex::Concat& n_) override;
    void visit(const mws::regex::ZeroOrOne& n_) override;
	void visit(const mws::regex::ZeroToMany& n_) override;
    void visit(const mws::regex::OneToMany& n_) override;
	void visit(const mws::regex::CharClass& n_) override;
	void visit(const mws::regex::Negate& n_) override;
    void visit(const mws::regex::RngConcat& n_) override;
	void visit(const mws::regex::Rng& n_) override;
    void visit(const mws::regex::CharClassSymbol& n_) override;

	std::wstring m_result;
};
