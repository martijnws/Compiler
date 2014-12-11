#pragma once

#include "DFAInfo.h"
#include "DFANode.h"
#include <cassert>
#include <set>

namespace mws {

template<typename Item>
class DFATraits;

template<>
class DFATraits<NFANode>
{
public:
    using Item = NFANode;
    using DFANode = mws::DFANode<Item>;

    static void c_closure(const NFANode* n_, const RangeKey& rk_, DFANode* d_)
    {
        assert(rk_._l != NFA::E);

        auto range = n_->_transitionMap.equal_range(rk_);

        for (auto itr = range.first; itr != range.second; ++itr)
        {
            // rk must be subset of target transition. That is, all chars in rk range
            // must fit this transition
            assert(rk_._l >= itr->first._l && rk_._h <= itr->first._h);

            auto n = itr->second;
            d_->insert(n);
        }
    }

    static std::size_t e_closure(NFANode::Map::const_iterator itr_, NFANode::Map::const_iterator end_, DFANode* d_)
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
            if (n->_transitionMap.size() == 0 || e_closure(range.first, range.second, d_) != n->_transitionMap.size())
            {
                d_->insert(n);
            }
        }

        return count;
    }

    static DFANode* e_closure(DFANode* d_)
    {
        for (auto n : d_->_items)
        {
            auto range = n->_transitionMap.equal_range(NFA::E);
            e_closure(range.first, range.second, d_);
        }

        return d_;
    }

    static std::set<RangeKey, RangeKey::Less> getTransitionCharSet(const DFANode* d_)
    {
        std::set<RangeKey, RangeKey::Less> rkSet;

        for (const auto& n : d_->_items)
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

    static DFATraits<NFANode>::DFANode* createStartNode(const NFANode* n_)
    {
        auto d = new DFANode();

        // only insert if n_ is important (has non-E transitions)
        for (const auto& kvpair : n_->_transitionMap)
        {
            if (kvpair.first._l != NFA::E)
            {
                d->insert(n_);
                break;
            }
        }

        auto range = n_->_transitionMap.equal_range(NFA::E);
        e_closure(range.first, range.second, d);

        return d;
    }
};

}