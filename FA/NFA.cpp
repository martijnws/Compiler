#pragma once

#include "NFA.h"

namespace mws {

void traverse(NFANode* n_, const std::function<void(NFANode*)>& func_, std::set<NFANode*>& visitSet_)
{
    for (const auto& kvpair : n_->_transitionMap)
    {
        auto n = kvpair.second;
        if (visitSet_.insert(n).second)
        {
            traverse(n, func_, visitSet_);
        }
    }

    // execute action postfix
    func_(n_);
}

void traverse(NFANode* n_, const std::function<void(NFANode*)>& func_)
{
    std::set<NFANode*> visitSet;
    visitSet.insert(n_);
    traverse(n_, func_, visitSet);
}

void makeTransitionMapRangeKeysDisjoint(NFANode* n_)
{
    std::vector<RangeKey> rkVec;
    for (const auto& kvpair : n_->_transitionMap)
    {
        rkVec.push_back(kvpair.first);
    }

    std::set<RangeKey, RangeKey::Less> rkSet = getDisjointRangeSet(rkVec);

    NFANode::Map transitionMap;
    for (const auto& kvpair : n_->_transitionMap)
    {
        const auto& rkSuper = kvpair.first;
        auto n = kvpair.second;

        auto itr = rkSet.find(rkSuper._l);
        assert(itr != rkSet.end());

        Char l = itr->_l;
        Char h = itr->_h;

        for ( ; itr != rkSet.end(); ++itr)
        {
            const auto& rkSub = *itr;
            assert(rkSub._l >= rkSuper._l);
            
            if (rkSub._h > rkSuper._h)
            {
                break;
            }

            h = rkSub._h;
            transitionMap.insert(std::make_pair(rkSub, n));
        }

        assert(l == rkSuper._l);
        assert(h == rkSuper._h);
    }

    n_->_transitionMap.swap(transitionMap);
}

}