#pragma once

#include "NFA.h"
#include <set>

namespace mws {

class DFAInfo
{
public:
    // This info tells us: on input _lexeme we can go to next DFANode containing _followPos as subset.
    DFAInfo() : _isNullable(false), _lexeme(NFA::E), _regexID(-1) {}

    using NodeSet = std::set<DFAInfo*>;

    bool accept() const
    {
        return _regexID != -1;
    }

    NodeSet  _firstPos;
    NodeSet  _lastPos;
    // used to construct the next DFANode (set of DFAInfo items)
    NodeSet  _followPos;
    bool     _isNullable;
    // represents transition.
    RangeKey _lexeme;

    std::size_t _regexID;
};

}
