#pragma once

#include "NFA.h"
#include <set>
#include <unordered_set>
#include <map>
#include <cassert>
#include <memory>

namespace mws {

class DFANode
{
public:
    using NFANodeSet = std::set<const NFANode*>;
    using DFANodeMap = std::map<char, DFANode*>;

    DFANode() : _hash(0){}

    void insert(const NFANode* nfaNode_)
    {
        _nfaNodes.insert(nfaNode_);
    }

    void calculateHash()
    {
        // not a tested and proven hash (its rubbish!) but good enough for now
        std::hash<const NFANode*> hash;
            
        for (const NFANode* n : _nfaNodes)
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
            if (lhs_->_nfaNodes.size() != rhs_->_nfaNodes.size())
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

            return lhs_->_nfaNodes == rhs_->_nfaNodes;
        }
    };

    DFANodeMap  _transitionMap;
    NFANodeSet  _nfaNodes;
    std::size_t _hash;
};

using DFANodeSet = std::unordered_set<DFANode*, DFANode::Hash, DFANode::Pred>;

void c_closure(const NFANode* n_, char c_, DFANode* d_)
{
    assert(c_ != NFA::E);

    auto range = n_->_transitionMap.equal_range(c_);

    for (auto itr = range.first; itr != range.second; ++itr)
    {
        const NFANode* n = itr->second;
        d_->insert(n);
    }
}

void e_closure(const NFANode* n_, DFANode* d_)
{
    auto range = n_->_transitionMap.equal_range(NFA::E);

    for (auto itr = range.first; itr != range.second; ++itr)
    {
        const NFANode* n = itr->second;
        d_->insert(n);
        e_closure(n, d_);
    }
}

DFANode* e_closure(DFANode* d_)
{
    for (const NFANode* n : d_->_nfaNodes)
    {
        e_closure(n, d_);
    }

    return d_;
}

DFANode* move(DFANode* dSrc_, char c_)
{
    DFANode* dDst = new DFANode();

    for (const NFANode* n : dSrc_->_nfaNodes)
    {
        c_closure(n, c_, dDst);
    }

    return dDst;
}

void convert(DFANode* dSrc_, DFANodeSet& dfaNodes_)
{
    // obtain all characters for which dSrc contains outgoing transitions
    std::set<char> charSet;
    for (const NFANode* n : dSrc_->_nfaNodes)
    {
        for (auto kvpair : n->_transitionMap)
        {
            if (kvpair.first != NFA::E)
            {
                charSet.insert(kvpair.first);
            }
        }
    }

    for (char c : charSet)
    {
        DFANode* dDst = e_closure(move(dSrc_, c));
        // this condition holds because we just checked each c has a transition for some n in d
        assert(!dDst->_nfaNodes.empty());

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

DFANode* convert(const NFANode* n_)
{
    DFANode* d = new DFANode();
    d->insert(n_);
    e_closure(d);

    DFANodeSet dfaNodes;
    d->calculateHash();
    dfaNodes.insert(d);
    convert(d, dfaNodes);

    return d;
}

bool match(const DFANode* dfa_, const NFANode* nEnd_, const std::string& str_)
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

    return d->_nfaNodes.find(nEnd_) != d->_nfaNodes.end();
}

bool simulate(const NFA& nfa_, const std::string& str_)
{
    // start state
    std::unique_ptr<DFANode> d(new DFANode());
    d->insert(nfa_._s);
    e_closure(d.get());

    for (char c : str_)
    {
        d.reset(e_closure(move(d.get(), c)));
    }

    return d->_nfaNodes.find(nfa_._f) != d->_nfaNodes.end();
}

}
