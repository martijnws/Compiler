#pragma once

#include "RangeKey.h"
#include <map>
#include <set>
#include <functional>

namespace mws {

class NFANode
{
public:
    using Map = std::multimap<RangeKey, NFANode*, RangeKey::Less>;

    NFANode() : _accept(false), _regex(0){}

    Map         _transitionMap;
    bool        _accept;
    std::size_t _regex;
};

class NFA
{
public:
    static const Char E = -1;

    NFANode* _s;
    NFANode* _f;
};


void traverse(NFANode* n_, const std::function<void(NFANode*)>& func_);

void makeTransitionMapRangeKeysDisjoint(NFANode* n_);

} //mws