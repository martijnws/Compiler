#pragma once

#include <CommonLib/CodePoint.h>
#include <CommonLib/String.h>
#include <algorithm>
#include <set>
#include <vector>
#include <cassert>

namespace mws {

class RangeKey
{
public:
    RangeKey(CodePoint c_) 
    :
        _l(c_), _h(c_)
    {
        
    }

    RangeKey(CodePoint l_, CodePoint h_) 
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

	String toString() const;

    CodePoint _l;
    CodePoint _h;
};

inline std::vector<RangeKey> getDisjointRangeKeys(const std::set<RangeKey>& rkSet_, const RangeKey& rk_)
{
    std::vector<RangeKey> rkVec;

    const auto& rkSuper = rk_;

	// The keys in rkSet are disjoint. A lookup in this set will find the unique rk that contains rkSuper._l
    auto itr = rkSet_.find(rkSuper._l);
    assert(itr != rkSet_.end());

    auto l = itr->_l;
    auto h = itr->_h;

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
	enum class Bound { Open, Close };

	using RangePair = std::pair<CodePoint, Bound>;

	// step 1: find all bounds (h or l) and sort them.
	// step 2: determine if a range exists between 2 bounds or not.
	// If open ranges - closed ranges > 0 then we are in a range,
	// otherwise we are in an open gap
    std::vector<RangePair> rkSortVec;
    for (const auto& rk : rkVec_)
    {
        rkSortVec.push_back(std::make_pair(rk._l, Bound::Open));
        rkSortVec.push_back(std::make_pair(rk._h, Bound::Close));
    }

    std::sort(rkSortVec.begin(), rkSortVec.end(), [](const RangePair& lhs_, const RangePair& rhs_)
    {
        return lhs_.first != rhs_.first ? lhs_.first < rhs_.first : lhs_.second < rhs_.second;
    });

    std::size_t cOpenRange = 0;
    std::set<RangeKey> rkSet;
    CodePoint l = 0;

    for (const auto& kvpair : rkSortVec)
    {
        auto bound = kvpair.first;
        auto type = kvpair.second;

        if (type == Bound::Open)
        {
            if (bound > l && cOpenRange > 0)
            {
                // up to but not including bound
                rkSet.insert(RangeKey(l, bound - 1));
            }

            ++cOpenRange;
            l = bound;
        }
        else //Bound::Close
        {
            --cOpenRange;

            // up to and including bound

            // TODO: refactor to get better asserts
            assert(l == 0 || bound >= l - 1);

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