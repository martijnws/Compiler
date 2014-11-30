#pragma once

#include "NFA.h"
#include "DFANode.h"
#include <unordered_set>
#include <memory>

namespace mws {

template<typename Item>
using DFANodeSet = std::unordered_set<DFANode<Item>*, typename DFANode<Item>::Hash, typename DFANode<Item>::Pred>;

template<typename Item>
DFANode<Item>* move(DFANode<Item>* dSrc_, char c_)
{
    auto dDst = new DFANode<Item>();

    for (const Item* n : dSrc_->_items)
    {
        DFATraits<Item>::c_closure(n, c_, dDst);
    }

    return dDst;
}

template<typename Item>
void convert(DFANode<Item>* dSrc_, DFANodeSet<Item>& dfaNodes_)
{
    // obtain all characters for which dSrc contains outgoing transitions
    std::set<char> charSet = DFATraits<Item>::getTransitionCharSet(dSrc_);

    for (char c : charSet)
    {
        DFANode<Item>* dDst = DFATraits<Item>::e_closure(move<Item>(dSrc_, c));
        // this condition holds because we just checked each c has a transition for some n in d
        assert(!dDst->_items.empty());

        dDst->calculateHash();
        auto res = dfaNodes_.insert(dDst);
        // Ensure we link to DFANode in the map. This node will be different from dDst if an equivalent DFANode is already in the map
        dSrc_->_transitionMap.insert(std::make_pair(c, *res.first));

        // There is an equivalent DFANode in the map already
        if (!res.second)
        {
            delete dDst;
            continue;
        }

        convert(dDst, dfaNodes_);
    }
}

template<typename Item>
DFANode<Item>* convert(const Item* n_)
{
    DFANode<Item>* d(DFATraits<Item>::createStartNode(n_));

    DFANodeSet<Item> dfaNodes;
    d->calculateHash();
    dfaNodes.insert(d);
    convert(d, dfaNodes);

    return d;
}

template<typename Item>
bool match(const DFANode<Item>* d_, const Item* nEnd_, const std::string& str_)
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

    return d_->_items.find(nEnd_) != d_->_items.end();
}

template<typename Item>
bool simulate(const Item* nBeg_, const Item* nEnd_, const std::string& str_)
{
    std::unique_ptr<DFANode<Item>> d(DFATraits<Item>::createStartNode(nBeg_));

    for (char c : str_)
    {
        d.reset(DFATraits<Item>::e_closure(move<Item>(d.get(), c)));
    }

    return d->_items.find(nEnd_) != d->_items.end();
}

}
