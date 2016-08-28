#pragma once

#include "CharClassVisitor.h"
#include "NFA.h"

namespace mws {

class NFABuilderVisitor
	:
	public ast::Visitor
{
public:
    NFABuilderVisitor(const std::set<RangeKey>& rkSet_);

	virtual void visit(const ast::Symbol& n_);
	virtual void visit(const ast::Choice& n_);
	virtual void visit(const ast::Concat& n_);
    virtual void visit(const ast::ZeroOrOne& n_);
	virtual void visit(const ast::ZeroToMany& n_);
    virtual void visit(const ast::OneToMany& n_);
	virtual void visit(const ast::CharClass& n_);
	
    NFANode* startState() 
    { 
        return _result._s;
    }

    NFANode* acceptState() 
    { 
        _result._f->_regexID = 0;
        return _result._f; 
    }

private:
    NFA _result;
    const std::set<RangeKey> _rkSet;
};

}
