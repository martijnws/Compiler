#include "stdafx.h"
#include "ToStringVisitor.h"
#include <SyntaxTreeLib\SyntaxNode.h>

void ToStringVisitor::visit(const mws::ast::Symbol& n_)
{
	wchar_t lexeme = n_.lexeme();
	m_result = lexeme;
}

void ToStringVisitor::visit(const mws::ast::Choice& n_) 
{
	m_result = L"|";
}

void ToStringVisitor::visit(const mws::ast::Concat& n_)
{
	m_result = L"&";
}

void ToStringVisitor::visit(const mws::ast::ZeroOrOne& n_)
{
	m_result = L"?";
}

void ToStringVisitor::visit(const mws::ast::ZeroToMany& n_)
{
	m_result = L"*";
}

void ToStringVisitor::visit(const mws::ast::OneToMany& n_)
{
	m_result = L"+";
}

void ToStringVisitor::visit(const mws::ast::CharClass& n_)
{
	m_result = L"[]";
}

void ToStringVisitor::visit(const mws::ast::Negate& n_)
{
	m_result = L"^";
}

void ToStringVisitor::visit(const mws::ast::RngConcat& n_)
{
	m_result = L"&";
}

void ToStringVisitor::visit(const mws::ast::Rng& n_)
{
	m_result = L"R"; 
}

void ToStringVisitor::visit(const mws::ast::CharClassSymbol& n_)
{
	wchar_t lexeme = n_.lexeme();
	m_result = lexeme;
}
