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

    static void e_closure(const NFANode* n_, DFANode* d_)
    {
        auto range = n_->_transitionMap.equal_range(NFA::E);

        for (auto itr = range.first; itr != range.second; ++itr)
        {
            auto n = itr->second;
            d_->insert(n);
            e_closure(n, d_);
        }
    }

    static DFANode* e_closure(DFANode* d_)
    {
        for (auto n : d_->_items)
        {
            e_closure(n, d_);
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
        d->insert(n_);
        e_closure(d);
        return d;
    }
};

}