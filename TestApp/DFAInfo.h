#pragma once

#include <set>

namespace mws {

class DFAInfo
{
public:
    using NodeSet = std::set<DFAInfo*>;

    NodeSet _firstPos;
    NodeSet _lastPos;
    NodeSet _followPos;
    bool    _isNullable;
    char    _lexeme;
};

}
