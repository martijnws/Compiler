#pragma once

#include "NFA.h"
#include <SyntaxTreeLib/Visitor.h>
#include <string>
#include <set>

namespace mws {

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

    NFANode* startState() 
    { 
        return _result._s;
    }

    NFANode* acceptState() 
    { 
        return _result._f; 
    }

private:
    std::set<char> _charClassSet;
    NFA            _result;
};

}

