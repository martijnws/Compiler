#include "stdafx.h"
#include "ToStringVisitor.h"
#include <RegexSyntaxTreeLib\SyntaxNode.h>
#include <CommonLib/Unicode.h>

void ToStringVisitor::visit(const mws::regex::Symbol& n_)
{
	wchar_t buf[4];
	const auto size = mws::common::utf::Encoder<wchar_t>::encode(n_.lexeme(), buf);
	buf[size] = L'\0';
	m_result = buf;
}

void ToStringVisitor::visit(const mws::regex::Choice& n_) 
{
	m_result = L"|";
}

void ToStringVisitor::visit(const mws::regex::Concat& n_)
{
	m_result = L"&";
}

void ToStringVisitor::visit(const mws::regex::ZeroOrOne& n_)
{
	m_result = L"?";
}

void ToStringVisitor::visit(const mws::regex::ZeroToMany& n_)
{
	m_result = L"*";
}

void ToStringVisitor::visit(const mws::regex::OneToMany& n_)
{
	m_result = L"+";
}

void ToStringVisitor::visit(const mws::regex::CharClass& n_)
{
	m_result = L"[]";
}

void ToStringVisitor::visit(const mws::regex::Negate& n_)
{
	m_result = L"^";
}

void ToStringVisitor::visit(const mws::regex::RngConcat& n_)
{
	m_result = L"&";
}

void ToStringVisitor::visit(const mws::regex::Rng& n_)
{
	m_result = L"R"; 
}

void ToStringVisitor::visit(const mws::regex::CharClassSymbol& n_)
{
	visit(static_cast<const mws::regex::Symbol&>(n_));
}
