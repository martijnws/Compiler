#include "Visitor.h"
#include "SyntaxNode.h"

namespace mws { namespace regex {

void Visitor::visit(const Symbol& n_)
{
	visit(static_cast<const Leaf&>(n_));	
}
void Visitor::visit(const Choice& n_) 
{
	visit(static_cast<const BinaryOp&>(n_));	
}
void Visitor::visit(const Concat& n_)
{
	visit(static_cast<const BinaryOp&>(n_));	
}
void Visitor::visit(const ZeroOrOne& n_)
{
	visit(static_cast<const UnaryOp&>(n_));	
}
void Visitor::visit(const ZeroToMany& n_)
{
	visit(static_cast<const UnaryOp&>(n_));	
}
void Visitor::visit(const OneToMany& n_)
{
	visit(static_cast<const UnaryOp&>(n_));	
}
void Visitor::visit(const CharClass& n_)
{
	visit(static_cast<const UnaryOp&>(n_));	
}
void Visitor::visit(const Negate& n_)
{
	visit(static_cast<const UnaryOp&>(n_));	
}
void Visitor::visit(const RngConcat& n_)
{
	visit(static_cast<const BinaryOp&>(n_));	
}
void Visitor::visit(const Rng& n_)
{
	visit(static_cast<const BinaryOp&>(n_));	
}
void Visitor::visit(const CharClassSymbol& n_)
{
	visit(static_cast<const Leaf&>(n_));	
}

void Visitor::visit(const Leaf& n_) {};
void Visitor::visit(const UnaryOp& n_) {};
void Visitor::visit(const BinaryOp& n_) {};

}}

