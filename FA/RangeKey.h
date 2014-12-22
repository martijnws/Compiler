#pragma once

#include "Char.h"
#include <algorithm>
#include <set>
#include <vector>
#include <cassert>

namespace mws {

class RangeKey
{
public:
    RangeKey(Char c_) 
    :
        _l(c_), _h(c_)
    {
        
    }

    RangeKey(Char l_, Char h_) 
    :
        _l(l_), _h(h_)
    {
        assert(_l <= _h);
    }

    friend bool operator == (const RangeKey& lhs_, const RangeKey& rhs_)
    {
        return lhs_._l == rhs_._l && lhs_._h == rhs_._h;
    }

   /* RangeKey intersection(const RangeKey& rhs_)
    {
        return RangeKey(std::max(_l, rhs_._l), std::min(_h, rhs_._h));
    }
*/
    friend bool operator < (const RangeKey& lhs_, const RangeKey& rhs_)
    {
        // equality as !(a < b || b < a) is not transitive! For example b == a-z, a-z == c, but b != c
        // However, if we ensure that every pair of RangeKeys is either equal (their _l and _h members are both equal)
        // or non overlapping then transitivity of equality holds for those elements. Furthermore under these conditions
        // we have a proper weak order (<=)
        return lhs_._h < rhs_._l;
    }

    Char _l;
    Char _h;
};

inline std::vector<RangeKey> getDisjointRangeKeys(const std::set<RangeKey>& rkSet_, const RangeKey& rk_)
{
    std::vector<RangeKey> rkVec;

    const auto& rkSuper = rk_;

    auto itr = rkSet_.find(rkSuper._l);
    assert(itr != rkSet_.end());

    Char l = itr->_l;
    Char h = itr->_h;

    for ( ; itr != rkSet_.end(); ++itr)
    {
        const auto& rkSub = *itr;
        assert(rkSub._l >= rkSuper._l);
            
        if (rkSub._h > rkSuper._h)
        {
            break;
        }

        h = rkSub._h;
        rkVec.push_back(rkSub);
    }

    assert(l == rkSuper._l);
    assert(h == rkSuper._h);

    return rkVec;
}

inline std::set<RangeKey> getDisjointRangeSet(const std::vector<RangeKey>& rkVec_)
{
    std::vector<std::pair<Char, Char>> rkSortVec;
    for (const auto& rk : rkVec_)
    {
        rkSortVec.push_back(std::make_pair(rk._l, '0'));
        rkSortVec.push_back(std::make_pair(rk._h, '1'));
    }

    std::sort(rkSortVec.begin(), rkSortVec.end(), [](const std::pair<Char, Char>& lhs_, const std::pair<Char, Char>& rhs_)
    {
        return lhs_.first != rhs_.first ? lhs_.first < rhs_.first : lhs_.second < rhs_.second;
    });

    std::size_t cOpenRange = 0;
    std::set<RangeKey> rkSet;
    Char l = 0;

    for (const auto& kvpair : rkSortVec)
    {
        Char bound = kvpair.first;
        Char type = kvpair.second;

        if (type == '0')
        {
            if (bound > l && cOpenRange > 0)
            {
                // up to but not including bound
                rkSet.insert(RangeKey(l, bound - 1));
            }

            ++cOpenRange;
            l = bound;
        }
        else
        {
            --cOpenRange;

            // up to and including bound

            // TODO: refactor to get better asserts
            assert(bound >= l - 1);

            if (l <= bound)
            {
                rkSet.insert(RangeKey(l, bound));
            }

            // if a range is open then this new be the new lower bound for the
            // next rangeKey to be inserted
            l = bound + 1;
        }

        assert(cOpenRange >= 0);
    }

    assert(!cOpenRange);

    return rkSet;
}

}