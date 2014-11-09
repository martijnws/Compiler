#pragma once

#include <SyntaxTreeLib/Visitor.h>
#include <string>
#include <unordered_map>
#include <set>

namespace mws {

class NFANode
{
public:
    typedef std::unordered_multimap<char, NFANode*> Map;

    Map _transitionMap;
};

class NFA
{
public:
    static const char E = -1;

    NFANode* _s;
    NFANode* _f;
};

class NFABuilderVisitor 
	:
	public mws::ast::Visitor
{
public:
	virtual void visit(const ast::Symbol& n_);
	virtual void visit(const ast::Choice& n_);
	virtual void visit(const ast::Concat& n_);
	virtual void visit(const ast::ZeroToMany& n_);
	virtual void visit(const ast::CharClass& n_);
	virtual void visit(const ast::Negate& n_);
    virtual void visit(const ast::RngConcat& n_);
	virtual void visit(const ast::Rng& n_);
    virtual void visit(const ast::CharClassSymbol& n_);

    NFA _result;

private:
    std::set<char> _charClassSet;
};

}

