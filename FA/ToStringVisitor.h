#pragma once

#include <RegexSyntaxTreeLib/Visitor.h>
#include <CommonLib/String.h>

namespace mws {

class ToStringVisitor 
	:
	public regex::Visitor
{
public:
	void visit(const regex::Symbol& n_) override;
	void visit(const regex::Choice& n_) override;
	void visit(const regex::Concat& n_) override;
	void visit(const regex::ZeroToMany& n_) override;
	void visit(const regex::CharClass& n_) override;
	void visit(const regex::Negate& n_) override;
    void visit(const regex::RngConcat& n_) override;
	void visit(const regex::Rng& n_) override;
    void visit(const regex::CharClassSymbol& n_) override;

	String _result;
};

};
