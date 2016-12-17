#pragma once

#include "NFA.h"
#include <CommonLib/TokenID.h>
#include <set>

namespace mws {

class DFAInfo
{
public:
    // This info tells us: on input _lexeme we can go to next DFANode containing _followPos as subset.

    using NodeSet = std::set<DFAInfo*>;

    bool accept() const
    {
        return _regexID != InvalidTokenID;
    }

    NodeSet  _firstPos;
    NodeSet  _lastPos;
    // used to construct the next DFANode (set of DFAInfo items)
    NodeSet  _followPos;
    bool     _isNullable = false;
    // represents transition.
    RangeKey _lexeme = NFA::E;
    TokenID  _regexID = InvalidTokenID;
};

}
