#pragma once

#include "NFA.h"
#include <set>
#include <unordered_set>
#include <map>
#include <cassert>
#include <memory>

namespace mws { namespace direct {

class DFANode
{
public:
    using DFAInfoSet = std::set<const DFAInfo*>;
    using DFANodeMap = std::map<char, DFANode*>;

    DFANode() : _hash(0){}

    void insert(const DFAInfo* item_)
    {
        _items.insert(item_);
    }

    void calculateHash()
    {
        // not a tested and proven hash (its rubbish!) but good enough for now
        std::hash<const DFAInfo*> hash;
            
        for (const DFAInfo* n : _items)
        {
            _hash <<= 1;
            _hash += hash(n);
            _hash >>= 1;
        }
    }

    class Hash
    {
    public:
     
        std::size_t operator()(const DFANode* d_) const
        {
            assert(d_->_hash != 0);
            return d_->_hash;
        }
    };

    class Pred
    {
    public:
        bool operator()(const DFANode* lhs_, const DFANode* rhs_) const
        {
            if (lhs_->_items.size() != rhs_->_items.size())
            {
                return false;
            }

            Hash hash;
            std::size_t hlhs = hash(lhs_);
            std::size_t hrhs = hash(rhs_);

            if (hlhs != hrhs)
            {
                return false;
            }

            return lhs_->_items == rhs_->_items;
        }
    };

    DFANodeMap  _transitionMap;
    DFAInfoSet  _items;
    std::size_t _hash;
};

using DFANodeSet = std::unordered_set<DFANode*, DFANode::Hash, DFANode::Pred>;

void c_closure(const DFAInfo* n_, char c_, DFANode* d_)
{
    assert(c_ != NFA::E);

    if (n_->_lexeme == c_)
    {
        d_->_items.insert(n_->_followPos.begin(), n_->_followPos.end());
    }
}

void e_closure(const DFAInfo* n_, DFANode* d_)
{
    
}

DFANode* e_closure(DFANode* d_)
{
    return d_;
}

DFANode* move(DFANode* dSrc_, char c_)
{
    DFANode* dDst = new DFANode();

    for (const DFAInfo* n : dSrc_->_items)
    {
        c_closure(n, c_, dDst);
    }

    return dDst;
}

std::set<char> getTransitionCharSet(const DFANode* d_)
{
    std::set<char> charSet;
    for (const DFAInfo* n : d_->_items)
    {
        assert(n->_lexeme != NFA::E);
        if (!n->_followPos.empty())
        {
            charSet.insert(n->_lexeme);
        }
    }

    return charSet;
}

void convert(DFANode* dSrc_, DFANodeSet& dfaNodes_)
{
    // obtain all characters for which dSrc contains outgoing transitions
    std::set<char> charSet = getTransitionCharSet(dSrc_);

    for (char c : charSet)
    {
        DFANode* dDst = e_closure(move(dSrc_, c));
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

DFANode* convert(const DFAInfo* n_)
{
    DFANode* d = new DFANode();
    d->_items.insert(n_->_firstPos.begin(), n_->_firstPos.end());

    DFANodeSet dfaNodes;
    d->calculateHash();
    dfaNodes.insert(d);
    convert(d, dfaNodes);

    return d;
}

bool match(const DFANode* dfa_, const DFAInfo* nEnd_, const std::string& str_)
{
    const DFANode* d = dfa_;

    for (char c : str_)
    {
        auto itr = d->_transitionMap.find(c);
        if (itr == d->_transitionMap.end())
        {
            return false;
        }

        d = itr->second;
    }

    return d->_items.find(nEnd_) != d->_items.end();
}

bool simulate(const DFAInfo* nBeg_, const DFAInfo* nEnd_, const std::string& str_)
{
    // start state
    std::unique_ptr<DFANode> d(new DFANode());
    d->_items.insert(nBeg_->_firstPos.begin(), nBeg_->_firstPos.end());

    for (char c : str_)
    {
        d.reset(e_closure(move(d.get(), c)));
    }

    return d->_items.find(nEnd_) != d->_items.end();
}

}} //NS mws::Direct
