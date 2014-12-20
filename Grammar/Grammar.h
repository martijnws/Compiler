#pragma once

#include "TokenStore.h"
#include "Handler.h"
#include <CommonLib/Buffer.h>
#include <cstdint>
#include <functional>
#include <vector>
#include <set>

namespace mws { namespace grammar {

class NT;
using Grammar = std::vector<NT>;

class GrammarSymbol;

using Parse      = std::function<void (Handler&, const Grammar&, const GrammarSymbol&, common::Buffer&, Token&)>;

using Action     = std::function<void (Handler&, const Token&, TokenStore&)>;


class GrammarSymbol
{
public:
    bool isSubGrammarStartSymbol() const
    {
        return !!_parse;
    }

	bool    _isTerminal;
    bool    _fetchNext;
	uint8_t _type;
	Action  _action;
    Parse   _parse;
};


class Production
{
public:
    Production()
    :
        _derivesEmpty(true)
    {
        
    }

    Production(Production&& rhs_) 
    : 
        _gsList(std::move(rhs_._gsList)), _first(std::move(rhs_._first)), _derivesEmpty(rhs_._derivesEmpty)
    {
    
    }

	Production(const std::initializer_list<GrammarSymbol>& gsList_)
	: 
        _gsList(gsList_.begin(), gsList_.end()), _derivesEmpty(gsList_.size() > 0)
	{
		
	}

	std::vector<GrammarSymbol> _gsList;
	std::set<uint8_t>          _first;
	bool                       _derivesEmpty;
};

class NT
{
public:
	NT(const std::string& name_, const std::initializer_list<Production>& prodList_)
	: 
        _prodList(prodList_.begin(), prodList_.end()), _name(name_)
	{
		
	}

	bool derivesEmpty() const
	{
		return std::any_of(_prodList.begin(), _prodList.end(), [](const Production& prod_){ return prod_._derivesEmpty; });
	}

	void getFirst(std::set<uint8_t>& firstSet_) const
	{
		for (const Production& prod : _prodList)
		{
			firstSet_.insert(prod._first.begin(), prod._first.end());
		}
	}

	std::vector<Production> _prodList;
	std::set<uint8_t>       _follow;
	std::string             _name;
};

inline Action no_op()
{
    static auto l_no_op = [](Handler& h_, const Token& t_,  TokenStore& store_){};
    return l_no_op;
}


inline GrammarSymbol t(uint32_t type_)
{ 
    return { true, true, type_, no_op(), Parse() }; 
}

inline GrammarSymbol t(uint32_t type_, bool fetchNext_)
{ 
    return { true, fetchNext_, type_, no_op(), Parse() }; 
}

inline GrammarSymbol t(uint32_t type_, Action action_) 
{ 
    return { true, true, type_, action_, Parse() }; 
}

inline GrammarSymbol n(uint32_t type_)
{ 
    return { false, true, type_, no_op(), Parse() }; 
}

inline GrammarSymbol n(uint32_t type_, Action action_) 
{ 
    return { false, true, type_, action_, Parse() }; 
}

inline GrammarSymbol n(uint32_t type_, Action action_, Parse parse_)
{
	return { false, true, type_, action_, parse_};
}


template<typename H>
Action inline a(void (H::*action_)())
{
    return [action_](Handler& h_, const Token& t_, TokenStore& store_) { (static_cast<H&>(h_).*action_)(); };
}

template<typename H>
Action inline a(void (H::*action_)(const Token& t_))
{
    return [action_](Handler& h_, const Token& t_, TokenStore& store_) { (static_cast<H&>(h_).*action_)(t_); };
}


Action inline a_set(uint8_t index_)
{
    return [index_](Handler& h_, const Token& t_, TokenStore& store_) { store_.put(index_, t_); };
}

template<typename H>
Action inline a_get(void (H::*action_)(const Token& t_), uint8_t index_)
{
    return [action_, index_](Handler& h_, const Token& t_, TokenStore& store_) { (static_cast<H&>(h_).*action_)(store_.get(index_)); };
}

}}