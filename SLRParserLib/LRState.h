#pragma once

#include <cstdint>
#include <map>
#include <set>

namespace mws { namespace td { namespace SLR {

class GrammarSymbol
{
public:
    uint8_t _type;
    bool    _isTerminal;

    friend bool operator < (const GrammarSymbol& lhs_, const GrammarSymbol& rhs_) //const
    {
        if (lhs_._isTerminal != rhs_._isTerminal)
        {
            return lhs_._isTerminal;
        }

        return lhs_._type < rhs_._type;
    }
};

template<typename Item>
class LRState
{
public:
    using Map = std::map<GrammarSymbol, LRState<Item>*>;

    Map             _transitionMap;
    std::set<Item>  _itemSetClosure;
    uint8_t         _label;
};

}}}

