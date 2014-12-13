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

    NFANode() : _regexID(-1){}

    bool accept() const
    {
        return _regexID != -1;
    }

    Map         _transitionMap;
    std::size_t _regexID;
};

class NFA
{
public:
    static const Char E = -1;

    NFANode* _s;
    NFANode* _f;
};

} //mws