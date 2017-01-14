#pragma once

#include "DFAInfo.h"
#include "DFANode.h"
#include <cassert>
#include <set>
#include <iterator>

namespace mws {

template<typename Item>
class DFATraits;

template<>
class DFATraits<NFANode>
{
public:
    using Item = NFANode;
   
    static void c_closure(const Item* n_, const RangeKey& rk_, std::set<const Item*>& itemSet_)
    {
        assert(rk_._l != NFA::E);

        auto range = n_->_transitionMap.equal_range(rk_);

        for (auto itr = range.first; itr != range.second; ++itr)
        {
            // rk must be subset of target transition. That is, all chars in rk range
            // must fit this transition
            assert(rk_._l >= itr->first._l && rk_._h <= itr->first._h);

            auto n = itr->second;
            itemSet_.insert(n);
        }
    }

    static std::size_t e_closure(NFANode::Map::const_iterator itr_, NFANode::Map::const_iterator end_, std::set<const Item*>& itemSet_)
    {
        std::size_t count = 0;

        for ( ; itr_ != end_; ++itr_)
        {
            ++count;
            auto n = itr_->second;

            auto range = n->_transitionMap.equal_range(NFA::E);
            
            // Optimization: n is only inserted if it is an 'important' state. If n contains only E transitions
            // it is dead wood which slows down processing

            // size == 0: must include n because no out transitions indicate n is a final state
            // size != E transition count: must include n because n contains non-E transitions (n is 'important')
            if (n->_transitionMap.size() == 0 || e_closure(range.first, range.second, itemSet_) != n->_transitionMap.size())
            {
                itemSet_.insert(n);
            }
        }

        return count;
    }

    static std::set<const Item*> e_closure(std::set<const Item*>& itemSet_)
    {
		const auto itemSet = itemSet_;
        for (auto n : itemSet)
        {
            auto range = n->_transitionMap.equal_range(NFA::E);
            e_closure(range.first, range.second, itemSet_);
        }

        return itemSet_;
    }

    static std::set<RangeKey> getTransitionCharSet(const std::set<const Item*>& itemSet_)
    {
        std::set<RangeKey> rkSet;

        for (const auto& n : itemSet_)
        {
            for (const auto& kvpair : n->_transitionMap)
            {
                const auto& rk = kvpair.first;

                if (rk._l != NFA::E)
                {
                    assert(rk._h != NFA::E);
                    rkSet.insert(rk);
                }
            }
        }

        return rkSet;
    }

    static std::set<const Item*> createStartNode(const Item* n_)
    {
        std::set<const Item*> itemSet;

        // only insert if n_ is important (has non-E transitions)
        for (const auto& kvpair : n_->_transitionMap)
        {
            if (kvpair.first._l != NFA::E)
            {
                itemSet.insert(n_);
                break;
            }
        }

        auto range = n_->_transitionMap.equal_range(NFA::E);
        e_closure(range.first, range.second, itemSet);

        return itemSet;
    }
};

}