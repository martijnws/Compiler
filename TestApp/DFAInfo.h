#pragma once

#include <set>
#include "NFA.h"

namespace mws {

class DFAInfo
{
public:
    DFAInfo() : _isNullable(false), _lexeme(NFA::E) {}

    using NodeSet = std::set<DFAInfo*>;

    NodeSet _firstPos;
    NodeSet _lastPos;
    NodeSet _followPos;
    bool    _isNullable;
    char    _lexeme;
};

}
