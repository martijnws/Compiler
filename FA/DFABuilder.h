#pragma once

#include "NFA.h"
#include "DFANode.h"
#include <unordered_map>
#include <memory>
#include <sstream>
#include <limits>

namespace mws {


template<typename Item>
using DFANodeMap = std::unordered_map<typename ItemSet<const Item*>::Ptr, DFANode*, typename ItemSet<const Item*>::Hash, typename ItemSet<const Item*>::Pred>;

template<typename Item>
TokenID getAcceptRegexID(const std::set<const Item*>& itemSet_)
{
	constexpr auto maxID = std::numeric_limits<TokenID>::max();
    auto regexID = maxID;

    for (auto n : itemSet_)
    {
        if (n->accept())
        {
            regexID = std::min(regexID, n->_regexID);
        }
    }

    return regexID != maxID ? regexID : InvalidTokenID;
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
    std::set<RangeKey> rkSet = DFATraits<Item>::getTransitionCharSet(srcItemSet_);

    for (const auto& rk : rkSet)
    {
        auto dstItemSet = new ItemSet<const Item*>(DFATraits<Item>::e_closure(move<Item>(srcItemSet_, rk)));

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
    auto itemSet = new ItemSet<const Item*>(DFATraits<Item>::createStartNode(n_));

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

template<typename Item, typename CharT>
bool match(const DFANode* d_, const Item* nAccept_, const CharT* str_)
{
	std::basic_stringstream<CharT> ss(str_);
	common::BufferT<CharT, 4094> buf(ss);
	return match_buf(d_, nAccept_, buf);
}

template<typename Item, typename CharT>
bool match(const DFANode* d_, const Item* nAccept_, const std::basic_string<CharT>& str_)
{
	std::basic_stringstream<CharT> ss(str_);
	common::BufferT<CharT, 4094> buf(ss);
	return match_buf(d_, nAccept_, buf);
}

template<typename Item, typename BufferT>
bool match_buf(const DFANode* d_, const Item* nAccept_, BufferT& buf_)
{
	for (auto cp = buf_.next(); buf_.valid(); cp = buf_.next())
	{
        auto itr = d_->_transitionMap.find(cp);
        if (itr == d_->_transitionMap.end())
        {
            return false;
        }

        d_ = itr->second;
    }

    return d_->_regexID == nAccept_->_regexID;
}

template<typename Item, typename CharT>
bool simulate(const Item* nBeg_, const Item* nAccept_, const CharT* str_)
{
	std::basic_stringstream<CharT> ss(str_);
	common::BufferT<CharT, 4094> buf(ss);
	return simulate_buf(nBeg_, nAccept_, buf);
}

template<typename Item, typename CharT>
bool simulate(const Item* nBeg_, const Item* nAccept_, const std::basic_string<CharT>& str_)
{
	std::basic_stringstream<CharT> ss(str_);
	common::BufferT<CharT, 4094> buf(ss);
	return simulate_buf(nBeg_, nAccept_, buf);
}

template<typename Item, typename BufferT>
bool simulate_buf(const Item* nBeg_, const Item* nAccept_, BufferT& buf_)
{
    std::set<const Item*> itemSet(DFATraits<Item>::createStartNode(nBeg_));

	for (auto cp = buf_.next(); buf_.valid(); cp = buf_.next())
    {
        itemSet.swap(DFATraits<Item>::e_closure(move<Item>(itemSet, cp)));
    }

    return itemSet.find(nAccept_) != itemSet.end();
}

}
