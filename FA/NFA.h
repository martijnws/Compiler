#pragma once

#include "RangeKey.h"
#include <CommonLib/CodePoint.h>
#include <CommonLib/TokenID.h>
#include <map>
#include <set>
#include <functional>

namespace mws {

class NFANode
{
public:
    using Map = std::multimap<RangeKey, NFANode*>;

    bool accept() const
    {
        return _regexID != InvalidTokenID;
    }

    Map     _transitionMap;
    TokenID _regexID = InvalidTokenID;
};

class NFA
{
public:
    static const CodePoint E = -1;

    NFANode* _s;
    NFANode* _f;
};

} //mws