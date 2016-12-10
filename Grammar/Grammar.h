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

using Action     = std::function<void (Handler&, const Token*, TokenStore&)>;

class GrammarSymbol
{
public:
	bool    _isTerminal;
	uint8_t _type;
	Action  _action;
};


class Production
{
public:
    Production() = default;

	Production(const Production& rhs_) = default;

    Production(Production&& rhs_) 
    : 
        _gsList(std::move(rhs_._gsList)), _first(std::move(rhs_._first)), _derivesEmpty(rhs_._derivesEmpty)
    {
    
    }

	Production(const std::initializer_list<GrammarSymbol>& gsList_)
	: 
        _gsList(gsList_.begin(), gsList_.end()), _derivesEmpty(gsList_.size() == 0)
	{
		
	}

	std::vector<GrammarSymbol> _gsList;
	std::set<uint8_t>          _first;
	bool                       _derivesEmpty = true;
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
		for (const auto& prod : _prodList)
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
    auto l_no_op = [](Handler& h_, const Token* t_,  TokenStore& store_){};
    return l_no_op;
}


template<typename Enum>
inline GrammarSymbol t(Enum type_)
{ 
    return { true, static_cast<uint8_t>(type_), no_op()  }; 
}


template<typename Enum>
inline GrammarSymbol t(Enum type_, Action action_) 
{ 
    return { true, static_cast<uint8_t>(type_), action_ }; 
}

template<typename Enum>
inline GrammarSymbol n(Enum type_)
{ 
    return { false, static_cast<uint8_t>(type_), no_op() }; 
}

template<typename Enum>
inline GrammarSymbol n(Enum type_, Action action_) 
{ 
    return { false, static_cast<uint8_t>(type_), action_ }; 
}



template<typename H>
Action inline a(void (H::*action_)())
{
    return [action_](Handler& h_, const Token* t_, TokenStore& store_) 
	{ 
		(static_cast<H&>(h_).*action_)(); 
	};
}

template<typename H, typename T>
Action inline a(void (H::*action_)(const T&))
{
    return [action_](Handler& h_, const Token* t_, TokenStore& store_) 
	{ 
		(static_cast<H&>(h_).*action_)(static_cast<const T&>(*t_)); 
	};
}

Action inline a_set(uint8_t index_)
{
    return [index_](Handler& h_, const Token* t_, TokenStore& store_) 
	{ 
		store_.put(index_, t_); 
	};
}

template<typename H, typename T>
Action inline a_get(void (H::*action_)(const T&), uint8_t index_)
{
    return [action_, index_](Handler& h_, const Token* t_, TokenStore& store_) 
	{ 
		(static_cast<H&>(h_).*action_)(static_cast<const T&>(*store_.get(index_))); 
	};
}

}}