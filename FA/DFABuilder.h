#pragma once

#include "NFA.h"
#include "DFANode.h"
#include <unordered_map>
#include <memory>

namespace mws {


template<typename Item>
using DFANodeMap = std::unordered_map<typename ItemSet<Item>::Ptr, DFANode*, typename ItemSet<Item>::Hash, typename ItemSet<Item>::Pred>;

template<typename Item>
std::size_t getAcceptRegexID(const std::set<const Item*>& itemSet_)
{
    std::size_t regexID = -1;
    for (auto n : itemSet_)
    {
        if (n->accept())
        {
            regexID = std::min(regexID, n->_regexID);
        }
    }

    return regexID;
}

template<typename Item>
std::set<const Item*> move(const std::set<const Item*>& srcItemSet_, const RangeKey& rk_)
{
    std::set<const Item*> dstItemSet;

    for (const Item* n : srcItemSet_)
    {
        DFATraits<Item>::c_closure(n, rk_, dstItemSet);
    }

    return dstItemSet;
}

template<typename Item>
void convert(DFANode* dSrc_, const std::set<const Item*> srcItemSet_, DFANodeMap<Item>& dfaNodes_)
{
    // obtain all disjoint ranges for which dSrc contains outgoing transitions
    std::set<RangeKey, RangeKey::Less> rkSet = DFATraits<Item>::getTransitionCharSet(srcItemSet_);

    for (const auto& rk : rkSet)
    {
        auto dstItemSet = new ItemSet<Item>(DFATraits<Item>::e_closure(move<Item>(srcItemSet_, rk)));

        // this condition holds because we just checked each c has a transition for some n in d
        assert(!dstItemSet->empty());

        auto res = dfaNodes_.insert(std::make_pair(dstItemSet, nullptr));
        // If we have an equivalent node in the map already, use it instead.
        DFANode* dDst = res.first->second;
        assert(res.second || dDst);

        if (res.second)
        {
            dDst = new DFANode(getAcceptRegexID(dstItemSet->_items));
            res.first->second = dDst;

            convert(dDst, dstItemSet->_items, dfaNodes_);
        }
       
        assert(dDst);
        dSrc_->_transitionMap.insert(std::make_pair(rk, dDst));
    }
}

template<typename Item>
DFANode* convert(const Item* n_)
{
    auto itemSet = new ItemSet<Item>(DFATraits<Item>::createStartNode(n_));

    auto d = new DFANode(getAcceptRegexID(itemSet->_items));

    DFANodeMap<Item> dfaNodes;
    dfaNodes.insert(std::make_pair(itemSet, d));
    
    convert(d, itemSet->_items, dfaNodes);

    for (auto& kvpair : dfaNodes)
    {
        delete kvpair.first;
    }

    return d;
}

template<typename Item>
bool match(const DFANode* d_, const Item* nAccept_, const std::string& str_)
{
    for (char c : str_)
    {
        auto itr = d_->_transitionMap.find(c);
        if (itr == d_->_transitionMap.end())
        {
            return false;
        }

        d_ = itr->second;
    }

    return d_->_regexID == nAccept_->_regexID;
}

template<typename Item>
bool simulate(const Item* nBeg_, const Item* nAccept_, const std::string& str_)
{
    std::set<const Item*> itemSet(DFATraits<Item>::createStartNode(nBeg_));

    for (char c : str_)
    {
        itemSet.swap(DFATraits<Item>::e_closure(move<Item>(itemSet, c)));
    }

    return itemSet.find(nAccept_) != itemSet.end();
}

}
