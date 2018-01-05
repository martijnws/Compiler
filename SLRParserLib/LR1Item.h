#pragma once

#include <Grammar/Grammar.h>
#include <FA/DFANode.h>
#include <cstdint>
#include <map>
#include <set>

namespace mws { namespace td { namespace SLR {

class LR1Item
{
public:
    uint8_t _nt;
    uint8_t _prod;
    uint8_t _dot;
	TokenID _lookAhead;

    const grammar::Production& prod(const grammar::Grammar& grammar_) const
    {
        assert(_nt < grammar_.size());
        assert(_prod < grammar_[_nt]._prodList.size());
        return grammar_[_nt]._prodList[_prod];
    }

    friend bool operator == (const LR1Item& lhs_, const LR1Item& rhs_)
    {
        return lhs_._nt == rhs_._nt && lhs_._prod == rhs_._prod && lhs_._dot == rhs_._dot && lhs_._lookAhead == rhs_._lookAhead;
    }

    friend bool operator < (const LR1Item& lhs_, const LR1Item& rhs_)
    {
        // sort on dot first for pretty printing (dot furthest to right on top)
        if (lhs_._dot != rhs_._dot)
        {
            return lhs_._dot > rhs_._dot;
        }

        if (lhs_._nt != rhs_._nt)
        {
            return lhs_._nt < rhs_._nt;
        }

        if (lhs_._prod != rhs_._prod)
		{
			return lhs_._prod < rhs_._prod;
		}

		return lhs_._lookAhead < rhs_._lookAhead;
    }

    class Hash
    {
    public:
        std::size_t operator ()(const mws::td::SLR::LR1Item& item_)
        {
            std::size_t h = 0;
            h += static_cast<std::size_t>(item_._nt)   << (8 + 8 + sizeof(TokenID) * 8);
            h += static_cast<std::size_t>(item_._prod) << (8 + sizeof(TokenID) * 8);
            h += static_cast<std::size_t>(item_._dot)  << (sizeof(TokenID) * 8);
			h += item_._lookAhead;
            return h;
        }
    };
};

}}}
