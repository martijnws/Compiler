#pragma once

#include "CharClassVisitor.h"
#include "NFA.h"

namespace mws {

class NFABuilderVisitor
	:
	public regex::Visitor
{
public:
    NFABuilderVisitor(const std::set<RangeKey>& rkSet_);

	void visit(const regex::Symbol& n_) override;
	void visit(const regex::Choice& n_) override;
	void visit(const regex::Concat& n_) override;
    void visit(const regex::ZeroOrOne& n_) override;
	void visit(const regex::ZeroToMany& n_) override;
    void visit(const regex::OneToMany& n_) override;
	void visit(const regex::CharClass& n_) override;
	
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
