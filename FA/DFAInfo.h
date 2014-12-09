#pragma once

#include <set>
#include "NFA.h"

namespace mws {

class DFAInfo
{
public:
    // This info tells us: on input _lexeme we can go to next DFANode containing _followPos as subset.
    DFAInfo() : _isNullable(false), _lexeme(NFA::E) {}

    using NodeSet = std::set<DFAInfo*>;

    NodeSet  _firstPos;
    NodeSet  _lastPos;
    // used to construct the next DFANode (set of DFAInfo items)
    NodeSet  _followPos;
    bool     _isNullable;
    // represents transition.
    RangeKey _lexeme;
};

}
