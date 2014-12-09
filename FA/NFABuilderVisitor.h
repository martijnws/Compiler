#pragma once

#include "FABuilderVisitorBase.h"
#include "NFA.h"

namespace mws {

class NFABuilderVisitor
	:
	public FABuilderVisitorBase
{
public:
	virtual void visit(const ast::Symbol& n_);
	virtual void visit(const ast::Choice& n_);
	virtual void visit(const ast::Concat& n_);
	virtual void visit(const ast::ZeroToMany& n_);
	virtual void visit(const ast::CharClass& n_);
	
    NFANode* startState() 
    { 
        return _result._s;
    }

    NFANode* acceptState() 
    { 
        _result._f->_accept = true;
        return _result._f; 
    }

private:
    NFA _result;
};

}
