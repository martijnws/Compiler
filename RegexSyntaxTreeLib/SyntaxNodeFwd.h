#pragma once

#include <SyntaxTreeLib/SyntaxNodeFwd.h>

namespace mws { namespace regex {

class Symbol;
class Choice;
class Concat;
class ZeroOrOne;
class ZeroToMany;
class OneToMany;
class CharClass;
class Negate;
class RngConcat;
class Rng;
class CharClassSymbol;

class Visitor;

using SyntaxNode = ast::SyntaxNode<Visitor>;
using Leaf       = ast::Leaf<Visitor>;
using UnaryOp    = ast::UnaryOp<Visitor>;
using BinaryOp   = ast::BinaryOp<Visitor>;

}}
