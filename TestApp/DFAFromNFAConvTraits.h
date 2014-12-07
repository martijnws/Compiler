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

    static void c_closure(const NFANode* n_, char c_, DFANode* d_)
    {
        assert(c_ != NFA::E);

        auto range = n_->_transitionMap.equal_range(c_);

        for (auto itr = range.first; itr != range.second; ++itr)
        {
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

    static std::set<char> getTransitionCharSet(const DFANode* d_)
    {
        std::set<char> charSet;
        for (auto n : d_->_items)
        {
            for (auto& kvpair : n->_transitionMap)
            {
                if (kvpair.first != NFA::E)
                {
                    charSet.insert(kvpair.first);
                }
            }
        }

        return charSet;
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